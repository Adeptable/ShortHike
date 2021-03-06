
// Copyright (c) 1999-2006 by Digital Mars
// All Rights Reserved
// written by Walter Bright
// www.digitalmars.com
// License for redistribution is by either the Artistic License
// in artistic.txt, or the GNU General Public License in gnu.txt.
// See the included readme.txt for details.

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "root.h"

#include "init.h"
#include "declaration.h"
#include "aggregate.h"
#include "mtype.h"
#include "attrib.h"
#include "template.h"
#include "id.h"

char *Declaration::mangle()
#if __DMC__
    __out(result)
    {
	int len = strlen(result);

	assert(len > 0);
	//printf("mangle: '%s' => '%s'\n", toChars(), result);
	for (int i = 0; i < len; i++)
	{
	    assert(result[i] == '_' || isalnum(result[i]) || result[i] & 0x80);
	}
    }
    __body
#endif
    {
	OutBuffer buf;
	char *id;
	Dsymbol *s;

	//printf("Declaration::mangle(this = %p, '%s', parent = '%s', linkage = %d)\n", this, toChars(), parent ? parent->toChars() : "null", linkage);
	if (!parent || parent->isModule())	// if at global scope
	{
	    // If it's not a D declaration, no mangling
	    switch (linkage)
	    {
		case LINKd:
		    break;

		case LINKc:
		case LINKwindows:
		case LINKpascal:
		case LINKcpp:
		    return ident->toChars();

		case LINKdefault:
		    error("forward declaration");
		    return ident->toChars();

		default:
		    fprintf(stdmsg, "'%s', linkage = %d\n", toChars(), linkage);
		    assert(0);
	    }
	}

	s = this;
	do
	{
	    //printf("s = %p, '%s', parent = %p\n", s, s->toChars(), s->parent);
	    if (s->ident)
	    {
		FuncDeclaration *fd = s->isFuncDeclaration();
		if (s != this && fd)
		{
		    id = fd->mangle();
		    buf.prependstring(id);
		    goto L1;
		}
		else
		{
		    id = s->ident->toChars();
		    int len = strlen(id);
		    char tmp[sizeof(len) * 3 + 1];
		    buf.prependstring(id);
		    sprintf(tmp, "%d", len);
		    buf.prependstring(tmp);
		}
	    }
	    else
		buf.prependstring("0");
	    s = s->parent;
	} while (s);

	buf.prependstring("_D");
    L1:
	//printf("deco = '%s'\n", type->deco);
	buf.writestring(type->deco);

	id = buf.toChars();
	buf.data = NULL;
	return id;
    }

char *FuncDeclaration::mangle()
#if __DMC__
    __out(result)
    {
	assert(strlen(result) > 0);
    }
    __body
#endif
    {
	if (isMain())
	    return "_Dmain";

	return Declaration::mangle();
    }

char *StructDeclaration::mangle()
{
    //printf("StructDeclaration::mangle() '%s'\n", toChars());
    return Dsymbol::mangle();
}


char *TypedefDeclaration::mangle()
{
    //printf("TypedefDeclaration::mangle() '%s'\n", toChars());
    return Dsymbol::mangle();
}


char *ClassDeclaration::mangle()
{
    Dsymbol *parentsave = parent;

    //printf("ClassDeclaration::mangle() %s.%s\n", parent->toChars(), toChars());

    /* These are reserved to the compiler, so keep simple
     * names for them.
     */
    if (ident == Id::TypeInfo   ||
	ident == Id::TypeInfo_Struct   ||
	ident == Id::TypeInfo_Class    ||
	ident == Id::TypeInfo_Typedef  ||
	ident == Id::Exception  ||
	this == object     ||
	this == classinfo  ||
	this == Module::moduleinfo ||
	memcmp(ident->toChars(), "TypeInfo_", 9) == 0
       )
	parent = NULL;

    char *id = Dsymbol::mangle();
    parent = parentsave;
    return id;
}


char *TemplateInstance::mangle()
{
    //printf("TemplateInstance::mangle() '%s'\n", toChars());
    return Dsymbol::mangle();
}



char *Dsymbol::mangle()
{
    OutBuffer buf;
    char *id;

    //printf("Dsymbol::mangle() '%s'\n", toChars());
    id = ident ? ident->toChars() : toChars();
    if (parent)
    {
	//printf("  parent = '%s', kind = '%s'\n", parent->mangle(), parent->kind());
	buf.writestring(parent->mangle());
    }
    buf.printf("%d%s", strlen(id), id);
    //buf.writestring("_");
    //buf.writestring(id);
    id = buf.toChars();
    buf.data = NULL;
    return id;
}


