#include "PapyrusArgs.h"


template <> void PackValue <void>(VMValue * dst, void * src, VirtualMachine * vm)
{
	dst->SetNone();
}

template <> void PackValue <UInt32>(VMValue * dst, UInt32 * src, VirtualMachine * vm)
{
	dst->SetInt(*src);
}

template <> void PackValue <SInt32>(VMValue * dst, SInt32 * src, VirtualMachine * vm)
{
	dst->SetInt(*src);
}

template <> void PackValue <float>(VMValue * dst, float * src, VirtualMachine * vm)
{
	dst->SetFloat(*src);
}

template <> void PackValue <bool>(VMValue * dst, bool * src, VirtualMachine * vm)
{
	dst->SetBool(*src);
}

template <> void PackValue <BSFixedString>(VMValue * dst, BSFixedString * src, VirtualMachine * vm)
{
	dst->SetString(src->c_str());
}

template <> void PackValue <VMVariable>(VMValue * dst, VMVariable * src, VirtualMachine * vm)
{
	src->PackVariable(dst);
}

void BindID(VMIdentifier ** identifier, void * srcData, VirtualMachine * vm, IObjectHandlePolicy * handlePolicy, UInt32 typeID)
{
	UInt32	unk = 0;

	VMObjectTypeInfo	* typeInfo = (*identifier)->m_typeInfo;
	if(typeInfo)
		typeInfo->AddRef();

	if(vm->Unk_0C(&typeInfo->m_typeName, &unk))
	{
		UInt64	handle = handlePolicy->Create(typeID, srcData);

		if(	handlePolicy->IsType(unk, handle) ||
			(handle == handlePolicy->GetInvalidHandle()))
		{
			CALL_MEMBER_FN(vm->GetObjectBindPolicy(), BindObject)(identifier, handle);
		}
	}

	if(typeInfo)
		typeInfo->Release();
}

void PackHandle(VMValue * dst, void * src, UInt32 typeID, VirtualMachine * vm)
{
	dst->SetNone();

	if(!src) return;

	VMObjectTypeInfo	* typeInfo = nullptr;

	// get class info
	if(vm->GetObjectTypeInfo(typeID, &typeInfo))
		if(typeInfo)
			typeInfo->Release();

	if(!typeInfo) return;

	IObjectHandlePolicy	* handlePolicy = vm->GetHandlePolicy();

	UInt64			handle = handlePolicy->Create(typeID, src);
	VMIdentifier	* identifier = NULL;

	// find existing identifier
	if(!vm->GetObjectIdentifier(handle, typeInfo->m_typeName, 0, &identifier, 0))	// Still broken right now, something wrong with this signature
	{
		if(vm->CreateObjectIdentifier(&typeInfo->m_typeName, &identifier))
		{
			if(identifier)
			{
				BindID(&identifier, src, vm, handlePolicy, typeID);
			}
		}
	}

	// copy the identifier out
	if(identifier)
	{
		VMValue	tempValue;
		tempValue.SetComplexType(typeInfo);
		CALL_MEMBER_FN(dst, Set)(&tempValue);
		dst->SetIdentifier(&identifier);
	}

	// release our reference
	if(identifier)
	{
		if(!identifier->DecrementLock())
		{
			identifier->Destroy();
		}
	}
}


//// VMValue -> type

template <> void UnpackValue <float>(float * dst, VMValue * src)
{
	switch(src->type)
	{
	case VMValue::kType_Int:
		*dst = src->data.i;
		break;

	case VMValue::kType_Float:
		*dst = src->data.f;
		break;

	case VMValue::kType_Bool:
		*dst = src->data.b;
		break;

	default:
		*dst = 0;
		break;
	}
}

template <> void UnpackValue <UInt32>(UInt32 * dst, VMValue * src)
{
	switch(src->type)
	{
	case VMValue::kType_Int:
		*dst = src->data.u;
		break;

	case VMValue::kType_Float:
		*dst = src->data.f;
		break;

	case VMValue::kType_Bool:
		*dst = src->data.b;
		break;

	default:
		*dst = 0;
		break;
	}
}

template <> void UnpackValue <SInt32>(SInt32 * dst, VMValue * src)
{
	switch(src->type)
	{
	case VMValue::kType_Int:
		*dst = src->data.u;
		break;

	case VMValue::kType_Float:
		*dst = src->data.f;
		break;

	case VMValue::kType_Bool:
		*dst = src->data.b;
		break;

	default:
		*dst = 0;
		break;
	}
}

template <> void UnpackValue <bool>(bool * dst, VMValue * src)
{
	switch(src->type)
	{
	case VMValue::kType_Int:
		*dst = src->data.u != 0;
		break;

	case VMValue::kType_Float:
		*dst = src->data.f != 0;
		break;

	case VMValue::kType_Bool:
		*dst = src->data.b;
		break;

	default:
		*dst = 0;
		break;
	}
}

template <> void UnpackValue <BSFixedString>(BSFixedString * dst, VMValue * src)
{
	const char	* data = NULL;

	if(src->type == VMValue::kType_String)
		data = src->data.str;

	CALL_MEMBER_FN(dst, Set)(data);
}

template <> void UnpackValue <VMVariable>(VMVariable * dst, VMValue * src)
{
	dst->UnpackVariable(src);
}


template <> void UnpackValue <VMArray<UInt32>>(VMArray<UInt32> * dst, VMValue * src)
{
	UnpackArray(dst, src, VMValue::kType_IntArray);
}

template <> void UnpackValue <VMArray<SInt32>>(VMArray<SInt32> * dst, VMValue * src)
{
	UnpackArray(dst, src, VMValue::kType_IntArray);
}

template <> void UnpackValue <VMArray<float>>(VMArray<float> * dst, VMValue * src)
{
	UnpackArray(dst, src, VMValue::kType_FloatArray);
}

template <> void UnpackValue <VMArray<bool>>(VMArray<bool> * dst, VMValue * src)
{
	UnpackArray(dst, src, VMValue::kType_BoolArray);
}

template <> void UnpackValue <VMArray<BSFixedString>>(VMArray<BSFixedString> * dst, VMValue * src)
{
	UnpackArray(dst, src, VMValue::kType_StringArray);
}

template <> void UnpackValue <VMArray<VMVariable>>(VMArray<VMVariable> * dst, VMValue * src)
{
	UnpackArray(dst, src, VMValue::kType_VariableArray);
}

void * UnpackHandle(VMValue * src, UInt32 typeID)
{
	if(!src->IsIdentifier()) return NULL;
	if(!src->data.id) return NULL;

	UInt64	handle = src->data.id->GetHandle();

	if(!(*g_objectHandlePolicy)->IsType(typeID, handle)) return NULL;
	if(!(*g_objectHandlePolicy)->Unk_03(handle)) return NULL;

	return (*g_objectHandlePolicy)->Resolve(typeID, handle);
}


template <> UInt64 GetTypeID <void>(VirtualMachine * vm)					{ return VMValue::kType_None; }
template <> UInt64 GetTypeID <UInt32>(VirtualMachine * vm)					{ return VMValue::kType_Int; }
template <> UInt64 GetTypeID <SInt32>(VirtualMachine * vm)					{ return VMValue::kType_Int; }
template <> UInt64 GetTypeID <int>(VirtualMachine * vm)						{ return VMValue::kType_Int; }
template <> UInt64 GetTypeID <float>(VirtualMachine * vm)					{ return VMValue::kType_Float; }
template <> UInt64 GetTypeID <bool>(VirtualMachine * vm)					{ return VMValue::kType_Bool; }
template <> UInt64 GetTypeID <BSFixedString>(VirtualMachine * vm)			{ return VMValue::kType_String; }
template <> UInt64 GetTypeID <VMVariable>(VirtualMachine * vm)				{ return VMValue::kType_Variable; }

template <> UInt64 GetTypeID <VMArray<UInt32>>(VirtualMachine * vm)			{ return VMValue::kType_IntArray; }
template <> UInt64 GetTypeID <VMArray<SInt32>>(VirtualMachine * vm)			{ return VMValue::kType_IntArray; }
template <> UInt64 GetTypeID <VMArray<int>>(VirtualMachine * vm)			{ return VMValue::kType_IntArray; }
template <> UInt64 GetTypeID <VMArray<float>>(VirtualMachine * vm)			{ return VMValue::kType_FloatArray; }
template <> UInt64 GetTypeID <VMArray<bool>>(VirtualMachine * vm)			{ return VMValue::kType_BoolArray; }
template <> UInt64 GetTypeID <VMArray<BSFixedString>>(VirtualMachine * vm)	{ return VMValue::kType_StringArray; }
template <> UInt64 GetTypeID <VMArray<VMVariable>>(VirtualMachine * vm)		{ return VMValue::kType_VariableArray; }

UInt64 GetTypeIDFromFormTypeID(UInt32 formTypeID, VirtualMachine * vm)
{
	UInt64 result = 0;
	VMObjectTypeInfo * typeInfo = nullptr;
	if(vm->GetObjectTypeInfo(formTypeID, &typeInfo))
	{
		result = (UInt64)typeInfo;
		typeInfo->Release();
	}

	return result;
}

UInt64 GetTypeIDFromStructName(const char * name, VirtualMachine * vm)
{
	UInt64 result = 0;
	VMStructTypeInfo * typeInfo = nullptr;
	BSFixedString structName(name);
	if(vm->GetStructTypeInfo(&structName, &typeInfo))
	{
		result = (UInt64)typeInfo;
		typeInfo->Release();
	}

	return result;
}
