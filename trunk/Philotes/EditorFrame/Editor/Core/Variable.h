
/********************************************************************
日期:		2012年2月21日
文件名: 	Variable.h
创建者:		王延兴
描述:		变量封装	
*********************************************************************/

#pragma once

#include "Functor.h"
#include "RefCountBase.h"
#include "XmlInterface.h"
#include "StlUtil.h"

inline const char* to_c_str( const char *str ) { return str; }
inline const char* to_c_str( const CString &str ) { return str; }

#define MAX_VAR_STRING_LENGTH 4096

struct	IVarEnumList;
struct	ISplineInterpolator;
class	CUsedResources;

struct IVariable : public CRefCountBase
{
	enum EType
	{
		UNKNOWN, 
		INT,     
		BOOL,    
		FLOAT,   
		VECTOR,  
		QUAT,    
		STRING,  
		ARRAY
	};

	enum EDataType
	{
		DT_SIMPLE = 0,
		DT_BOOLEAN,
		DT_PERCENT,
		DT_COLOR,
		DT_ANGLE,
	};

	enum EFlags
	{
		UI_DISABLED         = 1<<0,
		UI_BOLD             = 1<<1,
		UI_SHOW_CHILDREN    = 1<<2,
		UI_USE_GLOBAL_ENUMS = 1<<3,
		UI_INVISIBLE        = 1<<4,
	};

	typedef Functor1<IVariable*> OnSetCallback;

	// Store IGetCustomItems into IVariable's UserData and set datatype to 
	// DT_USERITEMCB
	// RefCounting is NOT handled by IVariable!
	struct IGetCustomItems : public CRefCountBase
	{
		struct SItem
		{
			SItem() {}
			SItem(const CString& name, const CString& desc="") : name(name), desc(desc) {}
			SItem(const char* name, const char* desc="") : name(name), desc(desc) {}
			CString name;
			CString desc;
		};
		virtual bool GetItems (IVariable* /* pVar */ , std::vector<SItem>& /* items */ , CString& /* outDialogTitle */ ) = 0;
		virtual bool UseTree() = 0;
		virtual const char* GetTreeSeparator() = 0;
	};

	virtual	const char*	GetName() const = 0;

	virtual void SetName( const CString &name ) = 0;

	virtual	const char*	GetHumanName() const = 0;
	
	virtual void SetHumanName( const CString &name ) = 0;

	virtual const char*	GetDescription() const = 0;
	
	virtual void SetDescription( const char *desc ) = 0;

	virtual	EType	GetType() const = 0;

	virtual	int	GetSize() const = 0;

	virtual	unsigned char	GetDataType() const = 0;

	virtual void SetDataType( unsigned char dataType ) = 0;

	virtual void SetUserData( void* pData ) = 0;

	virtual void* GetUserData() = 0;

	virtual	void SetFlags( int flags ) = 0;

	virtual	int  GetFlags() const = 0;

	/////////////////////////////////////////////////////////////////////////////
	// Set methods.
	/////////////////////////////////////////////////////////////////////////////
	virtual void Set( int value ) = 0;
	virtual void Set( bool value ) = 0;
	virtual void Set( float value ) = 0;
	virtual void Set( const Float3& value ) = 0;
	virtual void Set( const Quaternion& value ) = 0;
	virtual void Set( const CString &value ) = 0;
	virtual void Set( const char* value ) = 0;
	virtual void SetDisplayValue( const CString &value ) = 0;

	// Called when value updated by any means (including internally).
	virtual void OnSetValue(bool bRecursive) = 0;

	/////////////////////////////////////////////////////////////////////////////
	// Get methods.
	/////////////////////////////////////////////////////////////////////////////
	virtual void Get( int &value ) const		= 0;
	virtual void Get( bool &value ) const		= 0;
	virtual void Get( float &value ) const  	= 0;
	virtual void Get( Float3& value ) const		= 0;
	virtual void Get( Quaternion &value ) const	= 0;
	virtual void Get( CString &value ) const	= 0;
	virtual CString GetDisplayValue() const 	= 0;

	//////////////////////////////////////////////////////////////////////////
	// For vector parameters.
	//////////////////////////////////////////////////////////////////////////
	virtual int NumChildVars() const = 0;
	virtual IVariable* GetChildVar( int index ) const = 0;
	virtual void AddChildVar( IVariable *var ) = 0;
	virtual bool DeleteChildVar( IVariable* var, bool recursive = false ) = 0;
	virtual void DeleteAllChilds() = 0;


	//! Return cloned value of variable.
	virtual IVariable* Clone( bool bRecursive ) const = 0;

	//! Copy variable value from specified variable.
	//! This method executed always recursively on all sub hierarchy of variables,
	//! In Array vars, will never create new variables, only copy values of corresponding childs.
	//! @param fromVar Source variable to copy value from.
	virtual void CopyValue( IVariable *fromVar ) = 0;

	//////////////////////////////////////////////////////////////////////////
	// Value Limits.
	//////////////////////////////////////////////////////////////////////////
	//! Set value limits.
	virtual void SetLimits( float fMin, float fMax, float fStep = 0.f, bool bHardMin = true, bool bHardMax = true ) {}
	//! Get value limits.
	virtual void GetLimits( float& fMin, float& fMax, float& fStep, bool& bHardMin, bool& bHardMax ) {}
	void GetLimits( float& fMin, float& fMax )
	{
		float f;
		bool b;
		GetLimits( fMin, fMax, f, b, b );
	}

	//////////////////////////////////////////////////////////////////////////
	// Wire/Unwire variables.
	//////////////////////////////////////////////////////////////////////////
	//! Wire variable, wired variable will be changed when this var changes.
	virtual void Wire( IVariable *targetVar ) = 0;
	//! Unwire variable.
	virtual void Unwire( IVariable *targetVar ) = 0;

	//////////////////////////////////////////////////////////////////////////
	// Assign on set callback.
	//////////////////////////////////////////////////////////////////////////
	virtual void AddOnSetCallback( OnSetCallback func ) = 0;
	virtual void RemoveOnSetCallback( OnSetCallback func ) = 0;

	//////////////////////////////////////////////////////////////////////////
	//! Retrieve pointer to selection list used by variable.
	virtual IVarEnumList* GetEnumList() const = 0;
	virtual ISplineInterpolator* GetSpline(bool bCreate) const = 0;

	//////////////////////////////////////////////////////////////////////////
	//! Serialize variable to XML.
	//////////////////////////////////////////////////////////////////////////
	virtual void Serialize( XmlNodeRef node,bool load ) = 0;

	// From CObject, (not implemented)
	virtual void Serialize(CArchive& ar) {};

	//////////////////////////////////////////////////////////////////////////
	// Disables the update callbacks for certain operations in order to avoid
	// too many funcion calls when not needed.
	//////////////////////////////////////////////////////////////////////////
	virtual void EnableUpdateCallbacks(bool boEnable)=0;
};

// Smart pointer to this parameter.
typedef TSmartPtr<IVariable> IVariablePtr;

class	CVariableBase : public IVariable
{
public:
	~CVariableBase() {}

	void SetName( const CString &name ) { m_name = name; };
	
	const char*	GetName() const { return to_c_str(m_name); };

	const char*	GetHumanName() const
	{
		if (!m_humanName.IsEmpty())
			return m_humanName;
		return m_name;
	}
	void SetHumanName( const CString &name ) { m_humanName = name; }

	void SetDescription( const char *desc ) { m_description = desc; };
	
	const char*	GetDescription() const { return to_c_str(m_description); };

	EType GetType() const { return  IVariable::UNKNOWN; };
	int	GetSize() const { return sizeof(*this); };

	unsigned char GetDataType() const { return m_dataType; };
	void SetDataType( unsigned char dataType ) { m_dataType = dataType; }

	void SetFlags( int flags ) { m_flags = flags; }
	int  GetFlags() const { return m_flags; }

	void SetUserData( void* pData ){ m_pUserData = pData; }
	void* GetUserData(){ return m_pUserData; }

	void Set( int value )							{ assert(0); }
	void Set( bool value )						{ assert(0); }
	void Set( float value )						{ assert(0); }
	void Set( const Float3& value )			{ assert(0); }
	void Set( const Quaternion &value )			{ assert(0); }
	void Set( const CString &value )	{ assert(0); }
	void Set( const char* value)      { assert(0); }
	void SetDisplayValue( const CString &value )	{ Set(value); }

	void Get( int &value ) const 			{ assert(0); }
	void Get( bool &value ) const 		{ assert(0); }
	void Get( float &value ) const 		{ assert(0); }
	void Get( Float3& value ) const 		{ assert(0); }
	void Get( Quaternion& value ) const 		{ assert(0); }
	void Get( CString &value ) const 	{ assert(0); }
	CString GetDisplayValue() const 	{ CString val; Get(val); return val; }

	int NumChildVars() const { return 0; }
	IVariable* GetChildVar( int index ) const { return 0; }
	void AddChildVar( IVariable *var ) { assert(0); }; // Not supported.
	bool DeleteChildVar( IVariable *var, bool recursive /*= false*/ ) { return false; }
	void DeleteAllChilds() {};

	//////////////////////////////////////////////////////////////////////////
	void Wire( IVariable *var )
	{
		m_wiredVars.push_back(var);
	}
	//////////////////////////////////////////////////////////////////////////
	void Unwire( IVariable *var )
	{
		if (!var)
		{
			// Unwire all.
			m_wiredVars.clear();
		}
		else
		{
			stl::find_and_erase( m_wiredVars,var );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void AddOnSetCallback( OnSetCallback func )
	{
		m_onSetFuncs.push_back(func);
	}

	//////////////////////////////////////////////////////////////////////////
	void RemoveOnSetCallback( OnSetCallback func )
	{
		stl::find_and_erase( m_onSetFuncs,func );
	}

	void OnSetValue(bool bRecursive)
	{
		// If have wired variables or OnSet callback, process them.
		// Send value to wired variable.
		for (CVariableBase::WiredList::iterator it = m_wiredVars.begin(); it != m_wiredVars.end(); ++it)
		{
			// Copy value to wired vars.
			(*it)->CopyValue(this);
		}

		if (!m_boUpdateCallbacksEnabled)
		{
			return;
		}

		// Call on set callback.
		for (OnSetCallbackList::iterator it = m_onSetFuncs.begin(); it != m_onSetFuncs.end(); ++it)
		{
			// Call on set callback.
			(*it)(this);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void Serialize( XmlNodeRef node,bool load )
	{
		if (load)
		{
			CString data;
			if (node->getAttr(to_c_str(m_name),data))
			{
				Set( data );
			}
		}
		else
		{
			// Saving.
			CString str;
			Get( str );
			node->setAttr( to_c_str(m_name),to_c_str(str) );
		}
	}

	virtual IVarEnumList* GetEnumList() const { return 0; };
	virtual ISplineInterpolator* GetSpline(bool bCreate) const { return 0; }

	virtual void EnableUpdateCallbacks(bool boEnable){m_boUpdateCallbacksEnabled=boEnable;};
protected:
	// Constructor.
	CVariableBase() 
	{
		m_dataType = DT_SIMPLE;
		m_flags = 0;
		m_pUserData = 0;
		m_boUpdateCallbacksEnabled=true;
	}
	// Copy constructor.
	CVariableBase( const CVariableBase &var )
	{
		m_name = var.m_name;
		m_humanName = var.m_humanName;
		m_description = var.m_description;
		m_flags = var.m_flags;
		m_dataType = var.m_dataType;
		m_pUserData = var.m_pUserData;
		m_boUpdateCallbacksEnabled=true;
		// Never copy callback function or wired variables they are private to specific variable,
	}

protected:
	// Not allow.
	CVariableBase& operator=( const CVariableBase &var )
	{
		return *this;
	}

protected:
	//////////////////////////////////////////////////////////////////////////
	// Variables.
	//////////////////////////////////////////////////////////////////////////
	typedef std::vector<OnSetCallback> OnSetCallbackList;
	typedef std::vector<IVariablePtr> WiredList;

	CString m_name;
	CString m_humanName;
	CString m_description;

	//! Extended data (Extended data is never copied, it's always private to this variable).
	WiredList m_wiredVars;
	OnSetCallbackList m_onSetFuncs;

	//! Limited to 8 flags.
	unsigned char m_flags;
	unsigned char m_dataType;

	//! Optional userdata pointer
	void* m_pUserData;

	bool m_boUpdateCallbacksEnabled;
};

/**
**************************************************************************************
* CVariableArray implements variable of type array of IVariables.
***************************************************************************************
*/
class	CVariableArray : public CVariableBase
{
public:
	//! Get name of parameter.
	virtual	EType	GetType() const { return IVariable::ARRAY; };
	virtual	int		GetSize() const { return sizeof(CVariableArray); };

	//////////////////////////////////////////////////////////////////////////
	// Set methods.
	//////////////////////////////////////////////////////////////////////////
	virtual void Set( const CString &value )
	{
		if (m_strValue != value)
		{
			m_strValue = value;
			OnSetValue(false);
		}
	}
	void OnSetValue(bool bRecursive)
	{
		CVariableBase::OnSetValue(bRecursive);
		if (bRecursive)
		{
			for (Vars::iterator it = m_vars.begin(); it != m_vars.end(); ++it)
			{
				(*it)->OnSetValue(true);
			}
		}
	}
	
	virtual void Get( CString &value ) const { value = m_strValue; }

	IVariable* Clone( bool bRecursive ) const
	{
		CVariableArray *var = new CVariableArray(*this);

		for (size_t i = 0; i < m_vars.size(); i++)
		{
			var->m_vars[i] = m_vars[i]->Clone(bRecursive);
		}
		return var;
	}

	//////////////////////////////////////////////////////////////////////////
	void CopyValue( IVariable *fromVar )
	{
		assert(fromVar);
		if (fromVar->GetType() != IVariable::ARRAY)
			return;
		int numSrc = fromVar->NumChildVars();
		int numTrg = m_vars.size();
		for (int i = 0; i < numSrc && i < numTrg; i++)
		{
			// Copy Every child variable.
			m_vars[i]->CopyValue( fromVar->GetChildVar(i) );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	int NumChildVars() const { return m_vars.size(); }
	IVariable* GetChildVar( int index ) const
	{
		assert( index >= 0 && index < (int)m_vars.size() );
		return m_vars[index];
	}
	void AddChildVar( IVariable *var )
	{
		m_vars.push_back(var);
	}

	bool DeleteChildVar( IVariable *var, bool recursive /*=false*/ )
	{
		bool found = stl::find_and_erase( m_vars, var );    
		if (!found && recursive)
		{
			for (Vars::iterator it = m_vars.begin(); it != m_vars.end(); ++it)
			{
				if ((*it)->DeleteChildVar(var, recursive))
					return true;
			}
		}
		return found;
	}

	void DeleteAllChilds()
	{
		m_vars.clear();
	}

	//////////////////////////////////////////////////////////////////////////
	void Serialize( XmlNodeRef node,bool load )
	{
		if (load)
		{
			// Loading.
			CString name;
			for (Vars::iterator it = m_vars.begin(); it != m_vars.end(); ++it)
			{
				IVariable *var = *it;
				if (var->NumChildVars())
				{
					XmlNodeRef child = node->findChild(var->GetName());
					if (child)
						var->Serialize( child,load );
				}
				else
					var->Serialize( node,load );
			}
		}
		else
		{
			// Saving.
			for (Vars::iterator it = m_vars.begin(); it != m_vars.end(); ++it)
			{
				IVariable *var = *it;
				if (var->NumChildVars())
				{
					XmlNodeRef child = node->newChild(var->GetName());
					var->Serialize( child,load );
				}
				else
					var->Serialize( node,load );
			}
		}
	}

protected:
	typedef std::vector<IVariablePtr> Vars;
	Vars m_vars;
	//! Any string value displayed in properties.
	CString m_strValue;
};

/** var_type namespace includes type definitions needed for CVariable implementaton.
*/
namespace var_type
{
	//////////////////////////////////////////////////////////////////////////
	template <int TypeID,bool IsStandart,bool IsInteger,bool IsSigned>
	struct type_traits_base 
	{
		static int type() { return TypeID; };
		//! Return true if standart C++ type.
		static bool is_standart() { return IsStandart; };
		static bool is_integer() { return IsInteger; };
		static bool is_signed() { return IsSigned; };
	};

	template <class Type>
	struct type_traits : public type_traits_base<IVariable::UNKNOWN,false,false,false> {};

	// Types specialization.
	template<> struct type_traits<int>		: public type_traits_base<IVariable::INT,true,true,true> {};
	template<> struct type_traits<bool>		: public type_traits_base<IVariable::BOOL,true,true,false> {};
	template<> struct type_traits<float>	: public type_traits_base<IVariable::FLOAT,true,false,false> {};
	template<> struct type_traits<Float3>		: public type_traits_base<IVariable::VECTOR,false,false,false> {};
	template<> struct type_traits<Quaternion>		: public type_traits_base<IVariable::QUAT,false,false,false> {};
	template<> struct type_traits<CString>: public type_traits_base<IVariable::STRING,false,false,false> {};
	//////////////////////////////////////////////////////////////////////////

	/*
	//////////////////////////////////////////////////////////////////////////
	// Extended stream operators.
	//////////////////////////////////////////////////////////////////////////
	//! Put CString to out stream operator.
	inline std::ostream&	operator<<( std::ostream &stream, const CString &s )
	{
	stream << to_c_str(s);
	return stream;
	}

	//! Put Float3 to out stream operator.
	inline std::ostream&	operator<<( std::ostream &stream, const Float3& v )
	{
	stream << v.x << "," << v.y << "," << v.z;
	return stream;
	}

	//! Put Float3 to out stream operator.
	inline std::ostream&	operator<<( std::ostream &stream, const Angle& v )
	{
	stream << v.x << "," << v.y << "," << v.z;
	return stream;
	}

	//! Put Quaternion to out stream operator.
	inline std::ostream&	operator<<( std::ostream &stream, const Quaternion& q )
	{
	stream << q.w << "," << q.v.x << "," << q.v.y << "," << q.v.z;
	return stream;
	}

	//! Get CString from input stream operator.
	inline std::istream&	operator>>( std::istream &stream, CString& s )
	{
	// String is limited..
	char str[MAX_VAR_STRING_LENGTH];
	stream >> str;
	s = str;
	return stream;
	}

	static inline std::istream& comma(std::istream& stream)
	{
	stream >> std::ws;
	if (stream.peek() == ',')
	stream.get();
	stream >> std::ws;
	return stream;
	}

	//! Get Float3 from input stream operator.
	inline std::istream&	operator>>( std::istream &stream,Float3 &v )
	{
	v.x = v.y = v.z = 0;
	return stream >> v.x >> comma >> v.y >> comma >> v.z;
	}

	//! Get Quaternion from input stream operator.
	inline std::istream&	operator>>( std::istream &stream,Quaternion &q )
	{
	q.v.x = q.v.y = q.v.z = q.w = 0;
	return stream >> q.v.x >> comma >> q.v.y >> comma >> q.v.z >> comma >> q.w;
	}
	*/

	//////////////////////////////////////////////////////////////////////////
	// General one type to another type convertor class.
	//////////////////////////////////////////////////////////////////////////
	//	template <class From,class To>
	struct type_convertor
	{
		template <class From,class To>
		void operator()( const From &from,To &to ) const { assert(0); }

		void operator()( const int &from,int &to ) const { to = from; }
		void operator()( const int &from,bool &to ) const { to = from != 0; }
		void operator()( const int &from,float &to ) const { to = (float)from; }
		//////////////////////////////////////////////////////////////////////////
		void operator()( const bool &from,int &to ) const { to = from; }
		void operator()( const bool &from,bool &to ) const { to = from; }
		void operator()( const bool &from,float &to ) const { to = from; }
		//////////////////////////////////////////////////////////////////////////
		void operator()( const float &from,int &to ) const { to = (int)from; }
		void operator()( const float &from,bool &to ) const { to = from != 0; }
		void operator()( const float &from,float &to ) const { to = from; }

		void operator()( const Float3 &from,Float3 &to ) const { to = from; }
		void operator()( const Quaternion &from,Quaternion &to ) const { to = from; }
		void operator()( const CString &from,CString &to ) const { to = from; }

		void operator()( int value,CString &to ) const { to.Format("%d",value); };
		void operator()( bool value,CString &to ) const { to.Format("%d",(value)?(int)1:(int)0); };
		void operator()( float value,CString &to ) const { to.Format("%g",value); };
		void operator()( const Float3& value,CString &to ) const { to.Format("%g,%g,%g",value.x,value.y,value.z); };
		void operator()( const Quaternion& value,CString &to ) const { to.Format("%g,%g,%g,%g",value.w,value.x,value.y,value.z); };

		void operator()( const CString &from,int &value ) const { value = atoi((const char*)from); };
		void operator()( const CString &from,bool &value ) const { value = atoi((const char*)from) != 0; };
		void operator()( const CString &from,float &value ) const { value = (float)atof((const char*)from); };
		void operator()( const CString &from,Float3& value ) const
		{
			value = Float3(0,0,0);
			sscanf( (const char*)from,"%f,%f,%f",&value.x,&value.y,&value.z ); 
		};
		void operator()( const CString &from,Quaternion& value ) const
		{
			D3DXQuaternionIdentity(&value);
			sscanf( (const char*)from,"%f,%f,%f,%f",&value.w,&value.x,&value.y,&value.z ); 
		}
	};
	
	template <class Type>
	inline bool compare( const Type &arg1,const Type &arg2 )
	{
		return arg1 == arg2;
	};
	inline bool compare( const Float3 &v1,const Float3 &v2 )
	{
		return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
	}
	inline bool compare( const Quaternion &q1,const Quaternion &q2 )
	{
		return (q1 == q2) == TRUE;
	}
	inline bool compare( const char* s1,const char* s2 )
	{
		return strcmp(s1,s2) == 0;
	}
	
	template <class Type>
	inline void init( Type &val )
	{
		val = 0;
	}
	inline void init( Float3& val )	
	{	
		val.x = 0; val.y = 0;val.z = 0;	
	}
	inline void init( Quaternion &val )
	{
		val.x = 0; val.y = 0; val.z = 0; val.w = 0;
	}
	inline void init( const char* &val )
	{
		val = "";
	}
	inline void init( CString &val )
	{
	}
};

//////////////////////////////////////////////////////////////////////////
// Void variable does not contain any value.
//////////////////////////////////////////////////////////////////////////
class	CVariableVoid : public CVariableBase
{
public:
	CVariableVoid() {};
	virtual EType GetType() const { return  IVariable::UNKNOWN; };
	virtual IVariable* Clone( bool bRecursive ) const { return new CVariableVoid(*this); }
	virtual void CopyValue( IVariable *fromVar ) {};
protected:
	CVariableVoid( const CVariableVoid &v ) : CVariableBase(v) {};
};

//////////////////////////////////////////////////////////////////////////
template <class T>
class	CVariable : public CVariableBase
{
	typedef CVariable<T> Self;
public:
	// Constructor.
	CVariable()
	{
		// Initialize value to zero or empty string.
		var_type::init( m_valueDef );
		m_valueMin = 0;
		m_valueMax = 100;
		m_valueStep = 0.f;
		m_bHardMin = m_bHardMax = false;
	}

	//! Get name of parameter.
	virtual	EType	GetType() const { return  (EType)var_type::type_traits<T>::type(); };
	virtual	int		GetSize() const { return sizeof(T); };

	//////////////////////////////////////////////////////////////////////////
	// Set methods.
	//////////////////////////////////////////////////////////////////////////
	virtual void Set( int value )							{ SetValue(value); }
	virtual void Set( bool value )						{ SetValue(value); }
	virtual void Set( float value )						{ SetValue(value); }
	virtual void Set( const Float3& value )			{ SetValue(value); }
	virtual void Set( const Quaternion& value )			{ SetValue(value); }
	virtual void Set( const CString& value )	{ SetValue(value); }
	virtual void Set( const char* value)      { SetValue(CString(value)); }

	//////////////////////////////////////////////////////////////////////////
	// Get methods.
	//////////////////////////////////////////////////////////////////////////
	virtual void Get( int &value ) const 						{ GetValue(value); }
	virtual void Get( bool &value ) const 					{ GetValue(value); }
	virtual void Get( float &value ) const 					{ GetValue(value); }
	virtual void Get( Float3& value ) const 					{ GetValue(value); }
	virtual void Get( Quaternion& value ) const 					{ GetValue(value); }
	virtual void Get( CString& value ) const 				{ GetValue(value); }

	//////////////////////////////////////////////////////////////////////////
	// Limits.
	//////////////////////////////////////////////////////////////////////////
	virtual void SetLimits( float fMin, float fMax, float fStep = 0.f, bool bHardMin = true, bool bHardMax = true )
	{
		m_valueMin = fMin;
		m_valueMax = fMax;
		m_valueStep = fStep;
		m_bHardMin = bHardMin;
		m_bHardMax = bHardMax;
	}
	virtual void GetLimits( float& fMin, float& fMax, float& fStep, bool& bHardMin, bool& bHardMax )
	{
		fMin = m_valueMin;
		fMax = m_valueMax;
		fStep = m_valueStep;
		bHardMin = m_bHardMin;
		bHardMax = m_bHardMax;
	}

	//////////////////////////////////////////////////////////////////////////
	// Access operators.
	//////////////////////////////////////////////////////////////////////////

	//! Cast to held type.
	operator T const& () const { return m_valueDef; }

	//! Assign operator for variable.
	void operator=( const T& value ) { SetValue(value); }

	//////////////////////////////////////////////////////////////////////////
	IVariable* Clone( bool bRecursive ) const
	{
		Self *var = new Self(*this);
		return var;
	}

	//////////////////////////////////////////////////////////////////////////
	void CopyValue( IVariable *fromVar )
	{
		assert(fromVar);
		T val;
		fromVar->Get(val);
		SetValue(val);
	}

protected:

	//////////////////////////////////////////////////////////////////////////
	template <class P>
	void SetValue( const P &value )
	{
		T newValue = T();
		//var_type::type_convertor<P,T> convertor;
		var_type::type_convertor convertor;
		convertor( value,newValue );

		// compare old and new values.
		if (!var_type::compare(m_valueDef, newValue))
		{
			m_valueDef = newValue;
			OnSetValue(false);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	template <class P>
	void GetValue( P &value ) const
	{
		var_type::type_convertor convertor;
		convertor( m_valueDef,value );
	}

protected:
	T m_valueDef;

	// Min/Max value.
	float m_valueMin, m_valueMax, m_valueStep;
	unsigned char m_bHardMin : 1;
	unsigned char m_bHardMax : 1;
};

//////////////////////////////////////////////////////////////////////////

template <class T>
class	TVariableArray : public CVariable<T>
{
	typedef TVariableArray<T> Self;
public:
	TVariableArray() : CVariable<T>() {};

	TVariableArray( const Self &var ) : CVariable<T>(var)
	{}

	virtual	int		GetSize() const { return sizeof(Self); };

	IVariable* Clone( bool bRecursive ) const
	{
		Self *var = new Self(*this);
		for (Vars::const_iterator it = m_vars.begin(); it != m_vars.end(); ++it)
		{
			var->m_vars.push_back( (*it)->Clone(bRecursive) );
		}
		return var;
	}
	void CopyValue( IVariable *fromVar )
	{
		assert(fromVar);
		if (fromVar->GetType() != GetType())
			return;
		__super::CopyValue(fromVar);
		int numSrc = fromVar->NumChildVars();
		int numTrg = m_vars.size();
		for (int i = 0; i < numSrc && i < numTrg; i++)
		{
			// Copy Every child variable.
			m_vars[i]->CopyValue( fromVar->GetChildVar(i) );
		}
	}

	int NumChildVars() const { return m_vars.size(); }
	IVariable* GetChildVar( int index ) const
	{
		assert( index >= 0 && index < (int)m_vars.size() );
		return m_vars[index];
	}
	void AddChildVar( IVariable *var ) { m_vars.push_back(var); }
	void DeleteAllChilds() { m_vars.clear(); }

	void Serialize( XmlNodeRef node,bool load )
	{
		__super::Serialize( node,load );
		if (load)
		{
			CString name;
			for (Vars::iterator it = m_vars.begin(); it != m_vars.end(); ++it)
			{
				IVariable *var = *it;
				if (var->NumChildVars())
				{
					XmlNodeRef child = node->findChild(var->GetName());
					if (child)
						var->Serialize( child,load );
				}
				else
					var->Serialize( node,load );
			}
		}
		else
		{
			for (Vars::iterator it = m_vars.begin(); it != m_vars.end(); ++it)
			{
				IVariable *var = *it;
				if (var->NumChildVars())
				{
					XmlNodeRef child = node->newChild(var->GetName());
					var->Serialize( child,load );
				}
				else
					var->Serialize( node,load );
			}
		}
	}

protected:
	typedef std::vector<IVariablePtr> Vars;
	Vars m_vars;
};

struct IVarEnumList : public CRefCountBase
{
	virtual int				GetItemsCount() = 0;
	virtual const char*		GetItemName( int index ) = 0;
	virtual bool			NeedsSort() const = 0;
};
typedef TSmartPtr<IVarEnumList> IVarEnumListPtr;


template <class T>
class CVarEnumListBase : public IVarEnumList
{
public:
	virtual T NameToValue( const CString &name ) = 0;

	virtual CString ValueToName( T const& value ) = 0;

	virtual void AddItem( const CString &name,const T &value ) = 0;

	template <class TVal>
	static bool IsValueEqual( const TVal &v1,const TVal &v2 )
	{
		return v1 == v2;
	}
	static bool IsValueEqual( const CString &v1,const CString &v2 )
	{
		return stricmp(v1,v2) == 0;
	}

	virtual bool NeedsSort() const
	{
		return true;
	}

protected:
	virtual ~CVarEnumListBase() {};
	friend class TSmartPtr<CVarEnumListBase<T> >;
};

struct CUIEnumsDatabase_SEnum;

class CVarGlobalEnumList : public CVarEnumListBase<CString>
{
public:
	CVarGlobalEnumList(CUIEnumsDatabase_SEnum* pEnum) : m_pEnum(pEnum) {}
	CVarGlobalEnumList(const CString& enumName);

	//! Get the number of entries in enumeration.
	virtual int GetItemsCount();

	//! Get the name of specified value in enumeration.
	virtual const char* GetItemName( int index );

	virtual CString NameToValue(const CString& name);
	virtual CString ValueToName(const CString& value);

	//! Don't add anything to a global enum database
	virtual void AddItem(const CString &name,const CString &value ) {}

private:
	CUIEnumsDatabase_SEnum* m_pEnum;
};


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//! Selection list shown in combo box, for enumerated variable.
template <class T>
class CVarEnumList : public CVarEnumListBase<T>
{
public:
	struct Item {
		CString name;
		T value;
	};
	int GetItemsCount()										{ return m_items.size(); }
	const char* GetItemName( int index )
	{
		assert( index >= 0 && index < m_items.size() );
		return m_items[index].name;
	};

	//////////////////////////////////////////////////////////////////////////
	T NameToValue( const CString &name )
	{
		for (int i = 0; i < m_items.size(); i++)
		{
			if (name == m_items[i].name)
				return m_items[i].value;
		}
		return T();
	}

	//////////////////////////////////////////////////////////////////////////
	CString ValueToName( T const& value )
	{
		for (int i = 0; i < m_items.size(); i++)
		{
			if (IsValueEqual(value,m_items[i].value))
				return m_items[i].name;
		}
		return "";
	}

	//! Add new item to the selection.
	void AddItem( const CString &name,const T &value )
	{
		Item item;
		item.name = name;
		item.value = value;
		m_items.push_back(item);
	};

protected:
	~CVarEnumList() {};

private:
	std::vector<Item> m_items;
};

//////////////////////////////////////////////////////////////////////////////////
// CVariableEnum is the same as CVariable but it display enumerated values in UI
//////////////////////////////////////////////////////////////////////////////////
template <class T>
class	CVariableEnum : public CVariable<T>
{
public:
	//////////////////////////////////////////////////////////////////////////
	CVariableEnum() {};

	//! Assign operator for variable.
	void operator=( const T& value ) { SetValue(value); }

	//! Add new item to the enumeration.
	void AddEnumItem( const CString &name,const T &value )
	{
		if (GetFlags() & UI_USE_GLOBAL_ENUMS)  // don抰 think adding makes sense
			return;
		if (!m_enum)
			m_enum = new CVarEnumList<T>;
		m_enum->AddItem( name,value );
	};
	void SetEnumList( CVarEnumListBase<T> *enumList )
	{
		m_enum = enumList;
		OnSetValue(false);
	}
	IVarEnumList* GetEnumList() const
	{
		return m_enum;
	}
	//////////////////////////////////////////////////////////////////////////
	IVariable* Clone( bool bRecursive ) const
	{
		CVariableEnum<T> *var = new CVariableEnum<T>(*this);
		return var;
	}

	virtual CString GetDisplayValue() const
	{
		if (m_enum)
			return m_enum->ValueToName( m_valueDef );
		else
			return CVariable<T>::GetDisplayValue();
	}

	virtual void SetDisplayValue( const CString& value )
	{
		if (m_enum)
			Set( m_enum->NameToValue( value ) );
		else
			Set( value );
	}

protected:
	// Copy Constructor.
	CVariableEnum( const CVariableEnum<T> &var ) : CVariable<T>(var)
	{
		m_enum = var.m_enum;
	}
private:
	TSmartPtr<CVarEnumListBase<T> > m_enum;
};

//////////////////////////////////////////////////////////////////////////
// Smart pointers to variables.
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
template <class T,class TVar>
struct CSmartVariableBase
{
	typedef typename TVar VarType;

	CSmartVariableBase() { pVar = new VarType; }

	operator T const& () const { VarType *pV = pVar; return *pV; }
	//void operator=( const T& value ) { VarType *pV = pVar; *pV = value; }

	// Compare with pointer to variable.
	bool operator==( IVariable *pV ) { return pVar == pV; }
	bool operator!=( IVariable *pV ) { return pVar != pV; }

	operator VarType&() { VarType *pV = pVar; return *pV; }  // Cast to CVariableBase&
	VarType& operator*() const { return *pVar; }
	VarType* operator->(void) const { return pVar; }

	VarType* GetVar() const { return pVar; };

protected:
	TSmartPtr<VarType> pVar;
};

//////////////////////////////////////////////////////////////////////////
template <class T>
struct CSmartVariable : public CSmartVariableBase<T,CVariable<T> >
{
	void operator=( const T& value ) { VarType *pV = pVar; *pV = value; }
};

//////////////////////////////////////////////////////////////////////////
template <class T>
struct CSmartVariableArrayT : public CSmartVariableBase<T,TVariableArray<T> >
{
	void operator=( const T& value ) { VarType *pV = pVar; *pV = value; }
};

//////////////////////////////////////////////////////////////////////////
template <class T>
struct CSmartVariableEnum : public CSmartVariableBase<T,CVariableEnum<T> >
{
	void operator=( const T& value ) { VarType *pV = pVar; *pV = value; }
	void AddEnumItem( const CString &name,const T &value )
	{
		pVar->AddEnumItem( name,value );
	};
	void SetEnumList( CVarEnumListBase<T> *enumList )
	{
		pVar->EnableUpdateCallbacks(false);
		pVar->SetEnumList(enumList);
		pVar->EnableUpdateCallbacks(true);
	}
};

//////////////////////////////////////////////////////////////////////////
struct CSmartVariableArray
{
	typedef CVariableArray VarType;

	CSmartVariableArray() { pVar = new VarType; }

	//////////////////////////////////////////////////////////////////////////
	// Access operators.
	//////////////////////////////////////////////////////////////////////////
	operator VarType&() { VarType *pV = pVar; return *pV; }

	VarType& operator*() const { return *pVar; }
	VarType* operator->(void) const { return pVar; }

	VarType* GetVar() const { return pVar; };

private:
	TSmartPtr<VarType> pVar;
};
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
class CVarBlock : public CRefCountBase
{
public:
	// Dtor.
	~CVarBlock();
	//! Add parameter to block.
	void AddVariable( IVariable *var );
	void AddVariable( IVariable *pVar,const char *varName,unsigned char dataType=IVariable::DT_SIMPLE );
	// This used from smart variable pointer.
	void AddVariable( CVariableBase &var,const char *varName,unsigned char dataType=IVariable::DT_SIMPLE );
	void RemoveVariable( IVariable *var );

	// Returns true if var block contains specified variable.
	bool IsContainVariable( IVariable *pVar,bool bRecursive=true ) const;

	//! Find variable by name.
	IVariable* FindVariable( const char *name,bool bRecursive=true ) const;

	//! Return true if vriable block is empty (Does not have any vars).
	bool IsEmpty() const { return m_vars.empty(); }
	//! Returns number of variables in block.
	int GetVarsCount() const { return m_vars.size(); }
	//! Get pointer to stored variable by index.
	IVariable* GetVariable( int index )
	{
		assert( index >= 0 && index < (int)m_vars.size() );
		return m_vars[index];
	}
	// Clear all vars from VarBlock.
	void Clear() { m_vars.clear(); };

	//////////////////////////////////////////////////////////////////////////
	//! Clone var block.
	CVarBlock* Clone( bool bRecursive ) const;

	//////////////////////////////////////////////////////////////////////////
	//! Copy variable values from specifed var block.
	//! Do not create new variables, only copy values of existing ones.
	//! Should only be used to copy identical var blocks (eg. not Array type var copied to String type var)
	//! @param fromVarBlock Source variable block that contain copied values, must be identical to this var block.
	void CopyValues( CVarBlock *fromVarBlock );

	//////////////////////////////////////////////////////////////////////////
	//! Copy variable values from specifed var block.
	//! Do not create new variables, only copy values of existing ones.
	//! Can be used to copy slighly different var blocks, matching performed by variable name.
	//! @param fromVarBlock Source variable block that contain copied values.
	void CopyValuesByName( CVarBlock *fromVarBlock );

	void OnSetValues();

	//////////////////////////////////////////////////////////////////////////
	// Wire/Unwire other variable blocks.
	//////////////////////////////////////////////////////////////////////////
	//! Wire to other variable block.
	//! Only equialent VarBlocks can be wired (same number of variables with same type).
	//! Recursive wiring of array variables is supported.
	void Wire( CVarBlock *toVarBlock );
	//! Unwire var block.
	void Unwire( CVarBlock *varBlock );

	//! Add this callback to every variable in block (recursively).
	void AddOnSetCallback( IVariable::OnSetCallback func );
	//! Remove this callback from every variable in block (recursively).
	void RemoveOnSetCallback( IVariable::OnSetCallback func );

	//////////////////////////////////////////////////////////////////////////
	void Serialize( XmlNodeRef &vbNode,bool load );

	void ReserveNumVariables( int numVars );

	//////////////////////////////////////////////////////////////////////////
	//! Gather resources in this variable block.
	virtual void GatherUsedResources( CUsedResources &resources );

	void EnableUpdateCallbacks(bool boEnable);	
protected:
	IVariable* FindChildVar( const char *name,IVariable *pParentVar ) const;
	bool ContainChildVar( IVariable *pParentVar,IVariable *pVar ) const;
	void SetCallbackToVar( IVariable::OnSetCallback func,IVariable *pVar,bool bAdd );
	void WireVar( IVariable *src,IVariable *trg,bool bWire );
	void GatherUsedResourcesInVar( IVariable *pVar,CUsedResources &resources );

	typedef std::vector<IVariablePtr> Variables;
	Variables m_vars;
};

typedef TSmartPtr<CVarBlock> CVarBlockPtr;

//////////////////////////////////////////////////////////////////////////
class CVarObject :  public CRefCountBase
{
public:
	typedef IVariable::OnSetCallback VarOnSetCallback;

	CVarObject();
	~CVarObject();

	void Serialize( XmlNodeRef node,bool load );
	CVarBlock* GetVarBlock() const { return m_vars; };

	void AddVariable( CVariableBase &var,const CString &varName,VarOnSetCallback cb=NULL,unsigned char dataType=IVariable::DT_SIMPLE );
	void AddVariable( CVariableBase &var,const CString &varName,const CString &varHumanName,VarOnSetCallback cb=NULL,unsigned char dataType=IVariable::DT_SIMPLE );
	void AddVariable( CVariableArray &table,CVariableBase &var,const CString &varName,const CString &varHumanName,VarOnSetCallback cb=NULL,unsigned char dataType=IVariable::DT_SIMPLE );
	void ReserveNumVariables( int numVars );
	void RemoveVariable( IVariable *var );

	void EnableUpdateCallbacks(bool boEnable);
	void OnSetValues();
protected:

	void CopyVariableValues( CVarObject *sourceObject );

private:
	CVarBlockPtr m_vars;
};
