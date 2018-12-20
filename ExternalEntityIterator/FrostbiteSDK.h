#pragma once

#define OFFSET_CLIENTGAMECONTEXT 0x1443E0D80
#define OFFSET_ObfuscationMgr 0x144057520

//struct fb::ClientSoldierEntityClassInfo *fb__ClientSoldierEntity__c_TypeInfo;
#define OFFSET_ClientSoldierEntityClassInfo 0x1449E95D0

#define ValidPointer( pointer ) ( pointer != NULL && (DWORD_PTR)pointer >= 0x10000 && (DWORD_PTR)pointer < 0x000F000000000000 )

static 
_QWORD GetEntityList( _QWORD TypeInfo, DWORD GameWorldOffset )
{
	return TypeInfo + 0x20 * GameWorldOffset + 0x68;
}

void* DecryptPointer( DWORD64 EncryptedPtr, DWORD64 PointerKey );
bool __fastcall hashtable_find(hashtable<_QWORD>* table, hashtable_iterator<_QWORD>* iterator, _QWORD key);

namespace fb
{
	struct Vec4 { union {float v[4]; struct {float x;float y;float z;float w;}; }; };
	struct Matrix4x4 { union {Vec4 v[4]; float m[4][4]; struct {Vec4 right;Vec4 up;Vec4 forward;Vec4 trans;}; }; };
	typedef Matrix4x4 LinearTransform;

	template< class T > 
	class EncryptedPtr
	{
	private:
		void* m_encryptedPtr;
		void* m_pointerKey;
	public:
		T* GetPtr( )
		{
			return (T*)( DecryptPointer( (_QWORD)this->m_encryptedPtr, (_QWORD)this->m_pointerKey ) );
		}
	};
	
	class EntityIterator
	{
	public:
		class Element
		{
		public:
			EncryptedPtr<Element> m_pFlink;
			EncryptedPtr<Element> m_pBlink;
		};

		EncryptedPtr<Element> m_pFirst;
		EncryptedPtr<Element> m_pCurrent;
	};
};


void fb__GameEntity__GetTransform( void* _this, fb::LinearTransform* OutMatrix );
