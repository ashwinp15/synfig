/* === S Y N F I G ========================================================= */
/*!	\file value.cpp
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**
**	This package is free software; you can redistribute it and/or
**	modify it under the terms of the GNU General Public License as
**	published by the Free Software Foundation; either version 2 of
**	the License, or (at your option) any later version.
**
**	This package is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**	General Public License for more details.
**	\endlegal
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "value.h"
#include "general.h"
#include <ETL/stringf>
#include "canvas.h"
#include "valuenode_bone.h"
#include "gradient.h"
#include "bone.h"
#include "matrix.h"
#include "boneweightpair.h"



#include "vector.h"
#include "time.h"
#include "segment.h"
#include "color.h"

#endif

using namespace synfig;
using namespace std;
using namespace etl;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueBase::ValueBase():type(TYPE_NIL),data(0),ref_count(0),loop_(0)
{
}

ValueBase::ValueBase(Type x):
	type(x),
	data(0),
	loop_(0)
{
	switch(type)
	{
	case TYPE_BOOL:			data=static_cast<void*>(new bool());				break;
	case TYPE_INTEGER:		data=static_cast<void*>(new int());					break;
	case TYPE_ANGLE:		data=static_cast<void*>(new Angle());				break;
	case TYPE_VECTOR:		data=static_cast<void*>(new Vector());				break;
	case TYPE_TIME:			data=static_cast<void*>(new Time());				break;
	case TYPE_REAL:			data=static_cast<void*>(new Real());				break;
	case TYPE_COLOR:		data=static_cast<void*>(new Color());				break;
	case TYPE_SEGMENT:		data=static_cast<void*>(new Segment());				break;
	case TYPE_BLINEPOINT:	data=static_cast<void*>(new BLinePoint());			break;
	case TYPE_MATRIX:		data=static_cast<void*>(new Matrix());				break;
	case TYPE_BONE_WEIGHT_PAIR:	data=static_cast<void*>(new BoneWeightPair());	break;
	case TYPE_LIST:			data=static_cast<void*>(new list_type());			break;
	case TYPE_STRING:		data=static_cast<void*>(new String());				break;
	case TYPE_GRADIENT:		data=static_cast<void*>(new Gradient());			break;
	case TYPE_BONE:			data=static_cast<void*>(new Bone());				break;
	case TYPE_VALUENODE_BONE:	data=static_cast<void*>(new etl::handle<ValueNode_Bone>());	break;
	case TYPE_CANVAS:		data=static_cast<void*>(new etl::handle<Canvas>());	break;
	default:																	break;
	}
}

ValueBase::~ValueBase()
{
	clear();
}

const char*
ValueBase::get(const char*)const
{
	return get(String()).c_str();
}

#ifdef _DEBUG
String
ValueBase::get_string() const
{
	switch(type)
	{
	case TYPE_NIL:			return "Nil";
	case TYPE_BOOL:			return strprintf("Bool (%s)", get(bool()) ? "true" : "false");
	case TYPE_INTEGER:		return strprintf("Integer (%s)", get(int()));
	case TYPE_ANGLE:		return strprintf("Angle (%.2f)", Angle::deg(get(Angle())).get());

		// All types after this point are larger than 32 bits

	case TYPE_TIME:			return strprintf("Time (%s)", get(Time()).get_string().c_str());
	case TYPE_REAL:			return strprintf("Real (%f)", get(Real()));

		// All types after this point are larger than 64 bits

	case TYPE_VECTOR:		return strprintf("Vector (%f, %f)", get(Vector())[0], get(Vector())[1]);
	case TYPE_COLOR:		return strprintf("Color (%s)", get(Color()).get_string().c_str());
	case TYPE_SEGMENT:		return strprintf("Segment ((%f, %f) to (%f, %f))", get(Segment()).p1[0], get(Segment()).p1[1], get(Segment()).p2[0], get(Segment()).p2[1]);
	case TYPE_BLINEPOINT:	return strprintf("BLinePoint (%s)", get(BLinePoint()).get_vertex()[0], get(BLinePoint()).get_vertex()[1]);
	case TYPE_MATRIX:		return strprintf("Matrix (%s)",get(Matrix().get_string().c_str()));
	case TYPE_BONE_WEIGHT_PAIR:	return strprintf("Bone Weight Pair (%s)",get(BoneWeightPair()).get_string().c_str());

		// All types after this point require construction/destruction

	case TYPE_LIST:			return strprintf("List (%d elements)", get(list_type()).size());
	case TYPE_CANVAS:		return strprintf("Canvas (%s)", get(etl::loose_handle<Canvas>())->get_id().c_str());
	case TYPE_STRING:		return strprintf("String (%s)", get(String()).c_str());
	case TYPE_GRADIENT:		return strprintf("Gradient (%d cpoints)", get(Gradient()).size());
	case TYPE_BONE:			return strprintf("Bone (%s)", get(Bone()).get_string().c_str());
	case TYPE_VALUENODE_BONE:	return strprintf("ValueNodeBone (%s)", get(ValueNode_Bone::Handle())->get_string().c_str());
	default:				return "Invalid type";
	}
}
#endif	// _DEBUG

void
ValueBase::set(Canvas* x)
{
	clear();
	if(x && x->is_inline())
	{
		_set(etl::handle<Canvas>(x));
	}
	else
	{
		_set(etl::loose_handle<Canvas>(x));
	}
	assert(get(x)==x);
}

void
ValueBase::set(etl::loose_handle<Canvas> x)
{
	clear();
	if(x && x->is_inline())
		_set(etl::handle<Canvas>(x));
	else
		_set(etl::loose_handle<Canvas>(x));
	assert(get(x)==x);
}

void
ValueBase::set(etl::handle<Canvas> x)
{
	clear();
	if(x && x->is_inline())
		_set(etl::handle<Canvas>(x));
	else
		_set(etl::loose_handle<Canvas>(x));
	assert(get(x)==x);
}

void
ValueBase::set(const list_type &x)
{
	_set(x);
}

void
ValueBase::set(const char* x)
{
	_set(String(x));
}

void
ValueBase::set(char* x)
{
	_set(String(x));
}

bool
ValueBase::is_valid()const
{
	return type>TYPE_NIL && type<TYPE_END && ref_count;
}

bool
ValueBase::empty()const
{
	return !is_valid() || ((type==TYPE_LIST)?get_list().empty():false);
}

ValueBase::Type
ValueBase::get_contained_type()const
{
	if(type!=TYPE_LIST || empty())
		return TYPE_NIL;
	return get_list().front().get_type();
}

ValueBase&
ValueBase::operator=(const ValueBase& x)
{
	if(data!=x.data)
	{
		clear();
		type=x.type;
		data=x.data;
		ref_count=x.ref_count;
	}
	loop_=x.loop_;
	return *this;
}

void
ValueBase::clear()
{
	if(ref_count.unique() && data)
	{
		switch(type)
		{
		case TYPE_BOOL:			delete static_cast<bool*>(data);		break;
		case TYPE_INTEGER:		delete static_cast<int*>(data);			break;
		case TYPE_ANGLE:		delete static_cast<Angle*>(data);		break;
		case TYPE_TIME:			delete static_cast<Time*>(data);		break;
		case TYPE_REAL:			delete static_cast<Real*>(data);		break;
		case TYPE_VECTOR:		delete static_cast<Vector*>(data);		break;
		case TYPE_COLOR:		delete static_cast<Color*>(data);		break;
		case TYPE_SEGMENT:		delete static_cast<Segment*>(data);		break;
		case TYPE_BLINEPOINT:	delete static_cast<BLinePoint*>(data);	break;
		case TYPE_MATRIX:		delete static_cast<Matrix*>(data);		break;
		case TYPE_BONE_WEIGHT_PAIR:	delete static_cast<BoneWeightPair*>(data);	break;
		case TYPE_LIST:			delete static_cast<list_type*>(data);	break;
		case TYPE_CANVAS:
		{
			etl::handle<Canvas> canvas(get(etl::loose_handle<Canvas>()));
			if(canvas && canvas->is_inline())
				delete static_cast<etl::handle<Canvas>*>(data);
			else
				delete static_cast<etl::loose_handle<Canvas>*>(data);
			break;
		}
		case TYPE_STRING:		delete static_cast<String*>(data);		break;
		case TYPE_GRADIENT:		delete static_cast<Gradient*>(data);	break;
		case TYPE_BONE:			delete static_cast<Bone*>(data);		break;
		case TYPE_VALUENODE_BONE:	delete static_cast<etl::handle<ValueNode_Bone>*>(data);		break;
		default:
			break;
		}
	}

	ref_count.detach();
	data=0;
	type=TYPE_NIL;
}

String
ValueBase::type_name(Type id)
{
	// don't internationalize these type names - they're used in .sif files
	switch(id)
	{
	case TYPE_BOOL:			return N_("bool");
	case TYPE_INTEGER:		return N_("integer");
	case TYPE_ANGLE:		return N_("angle");
	case TYPE_TIME:			return N_("time");
	case TYPE_REAL:			return N_("real");
	case TYPE_VECTOR:		return N_("vector");
	case TYPE_COLOR:		return N_("color");
	case TYPE_SEGMENT:		return N_("segment");
	case TYPE_BLINEPOINT:	return N_("bline_point");
	case TYPE_MATRIX:		return N_("matrix");
	case TYPE_BONE_WEIGHT_PAIR:	return N_("bone_weight_pair");
	case TYPE_LIST:			return N_("list");
	case TYPE_CANVAS:		return N_("canvas");
	case TYPE_STRING:		return N_("string");
	case TYPE_GRADIENT:		return N_("gradient");
	case TYPE_BONE:			return N_("bone_object");
	case TYPE_VALUENODE_BONE:	return N_("bone_valuenode");
	case TYPE_NIL:			return N_("nil");
	default:
		break;
	}
	synfig::warning("Encountered unknown ValueBase with a Type of %d",id);
//	assert(0);
	return "UNKNOWN";
}

String
ValueBase::type_local_name(Type id)
{
	// don't internationalize these type names - they're using in .sif files
	switch(id)
	{
		/* TRANSLATORS: this is the name of a type -- see http://synfig.org/Types */
	case TYPE_BOOL:			return _("Bool");
		/* TRANSLATORS: this is the name of a type -- see http://synfig.org/Types */
	case TYPE_INTEGER:		return _("Integer");
		/* TRANSLATORS: this is the name of a type -- see http://synfig.org/Types */
	case TYPE_ANGLE:		return _("Angle");
		/* TRANSLATORS: this is the name of a type -- see http://synfig.org/Types */
	case TYPE_TIME:			return _("Time");
		/* TRANSLATORS: this is the name of a type -- see http://synfig.org/Types */
	case TYPE_REAL:			return _("Real");
		/* TRANSLATORS: this is the name of a type -- see http://synfig.org/Types */
	case TYPE_VECTOR:		return _("Vector");
		/* TRANSLATORS: this is the name of a type -- see http://synfig.org/Types */
	case TYPE_COLOR:		return _("Color");
		/* TRANSLATORS: this is the name of a type -- see http://synfig.org/Types */
	case TYPE_SEGMENT:		return _("Segment");
		/* TRANSLATORS: this is the name of a type -- see http://synfig.org/Types */
	case TYPE_BLINEPOINT:	return _("Bline Point");
		/* TRANSLATORS: this is the name of a type -- see http://synfig.org/Types */
	case TYPE_MATRIX:		return _("Matrix");
		/* TRANSLATORS: this is the name of a type -- see http://synfig.org/Types */
	case TYPE_BONE_WEIGHT_PAIR:	return _("Bone Weight Pair");
		/* TRANSLATORS: this is the name of a type -- see http://synfig.org/Types */
	case TYPE_LIST:			return _("List");
		/* TRANSLATORS: this is the name of a type -- see http://synfig.org/Types */
	case TYPE_CANVAS:		return _("Canvas");
		/* TRANSLATORS: this is the name of a type -- see http://synfig.org/Types */
	case TYPE_STRING:		return _("String");
		/* TRANSLATORS: this is the name of a type -- see http://synfig.org/Types */
	case TYPE_GRADIENT:		return _("Gradient");
		/* TRANSLATORS: this is the name of a type -- see http://synfig.org/Types */
	case TYPE_BONE:			return _("Bone Object");
		/* TRANSLATORS: this is the name of a type -- see http://synfig.org/Types */
	case TYPE_VALUENODE_BONE:	return _("Bone ValueNode");
		/* TRANSLATORS: this is the name of a type -- see http://synfig.org/Types */
	case TYPE_NIL:			return _("Nil");
	default:
		break;
	}
	synfig::warning("Encountered unknown ValueBase with a Type of %d",id);
//	assert(0);
	return _("UNKNOWN");
}

ValueBase::Type
ValueBase::ident_type(const String &str)
{
	if(str=="nil" ||
	   str=="null")				return TYPE_NIL;
	else if(str=="time" ||
			str==_("time"))		return TYPE_TIME;
	else if(str=="real" ||
			str=="float" ||
			str==_("real"))		return TYPE_REAL;
	else if(str=="integer" ||
			str=="int" ||
			str==_("integer"))	return TYPE_INTEGER;
	else if(str=="bool" ||
			str==_("bool"))		return TYPE_BOOL;
	else if(str=="angle" ||
			str=="degrees" ||
			str=="radians" ||
			str=="rotations")	return TYPE_ANGLE;
	else if(str=="vector" ||
			str=="point")		return TYPE_VECTOR;
	else if(str=="color")		return TYPE_COLOR;
	else if(str=="string")		return TYPE_STRING;
	else if(str=="canvas")		return TYPE_CANVAS;
	else if(str=="list")		return TYPE_LIST;
	else if(str=="segment")		return TYPE_SEGMENT;
	else if(str=="gradient")	return TYPE_GRADIENT;
	else if(str=="bone_object")	return TYPE_BONE;
	else if(str=="bone_valuenode")	return TYPE_VALUENODE_BONE;
	else if(str=="bline_point" ||
			str=="blinepoint")	return TYPE_BLINEPOINT;
	else if(str=="matrix")		return TYPE_MATRIX;
	else if(str=="bone_weight_pair")	return TYPE_BONE_WEIGHT_PAIR;

	return TYPE_NIL;
}

bool
ValueBase::operator==(const ValueBase& rhs)const
{
	if(get_type()!=rhs.get_type())
		return false;
	if(data==rhs.data)
		return true;

	switch(get_type())
	{
	case TYPE_TIME:			   return get(Time()).is_equal(rhs.get(Time()));
	case TYPE_REAL:			   return abs(get(Real())-rhs.get(Real()))<=0.00000000000001;
	case TYPE_INTEGER:		   return get(int())==rhs.get(int());
	case TYPE_BOOL:			   return get(bool())==rhs.get(bool());
	case TYPE_ANGLE:		   return get(Angle())==rhs.get(Angle());
	case TYPE_VECTOR:		   return get(Vector()).is_equal_to(rhs.get(Vector()));
	case TYPE_COLOR:		   return get(Color())==rhs.get(Color());
	case TYPE_STRING:		   return get(String())==rhs.get(String());
	case TYPE_CANVAS:		   return get(Canvas::LooseHandle())==rhs.get(Canvas::LooseHandle());
	case TYPE_LIST:			   return get_list()==rhs.get_list();
	case TYPE_VALUENODE_BONE:  return get(ValueNode_Bone::Handle())==rhs.get(ValueNode_Bone::Handle());
	case TYPE_SEGMENT:		// return get(Segment())==rhs.get(Segment());
	case TYPE_GRADIENT:		// return get(Gradient())==rhs.get(Gradient());
	case TYPE_BONE:			// return get(Bone())==rhs.get(Bone());
	case TYPE_BLINEPOINT:	// return get(BLinePoint())==rhs.get(BLinePoint());
	case TYPE_MATRIX:		// return get(Matrix())==rhs.get(Matrix());
	case TYPE_BONE_WEIGHT_PAIR:	// return get(BoneWeightPair())==rhs.get(BoneWeightPair());
	case TYPE_NIL:
	default:				   return false;
	}
	return false;
}
