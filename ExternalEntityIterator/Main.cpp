#include <Windows.h>
#include <stdio.h>
#include "RemoteProcessManager.h"
#include "DecodeRemotePointer.h"
#include "ObfuscationMgr.h"
#include "FrostbiteSDK.h"

int main()
{
	SetConsoleTitleA("ExternalEntityIterator");
	RPM = new RemoteProcessManager();

	DWORD dwProcId = RPM->GetProcessId("bfv.exe");
	printf("-> bfv found %u\n",dwProcId);
	if ( RPM->OpenProcess( dwProcId ) != true )
	{
		printf("-> failed to access process!\n");
		system("pause");
		return ERROR_SUCCESS;
	}
	else
		printf("-> opening process\n");

	_QWORD ClientGameContext = NULL;
	RPM->Read( OFFSET_CLIENTGAMECONTEXT, &ClientGameContext, sizeof(_QWORD) );
	printf("ClientGameContext: 0x%I64X\n", ClientGameContext);

	_QWORD ClientLevel = NULL;
	RPM->Read( ClientGameContext + 0x30, &ClientLevel, sizeof(_QWORD) );
	printf("ClientLevel: 0x%I64X\n", ClientLevel);

	_QWORD ClientGameWorld = NULL;
	RPM->Read( ClientLevel + 0x118, &ClientGameWorld, sizeof(_QWORD) );
	printf("ClientGameWorld: 0x%I64X\n", ClientGameWorld);

	DWORD GameWorldOffset = NULL;
	RPM->Read( ClientGameWorld + 0x28, &GameWorldOffset, sizeof(DWORD) );
	printf("GameWorldOffset: 0x%X\n", GameWorldOffset);

	_QWORD ClassInfo = OFFSET_ClientSoldierEntityClassInfo;

	_QWORD EntityList = GetEntityList( ClassInfo, GameWorldOffset );

	fb::EntityIterator<_QWORD> iterator = {};
	RPM->Read( EntityList, &iterator, sizeof(fb::EntityIterator<_QWORD>) );

	fb::EntityIterator<_QWORD>::Element* pElement = iterator.m_pFirst.GetPtr();
	printf("\n\nFirst Element: 0x%I64X\n\n",pElement);
	if (ValidPointer(pElement))
	{
		printf("----------------------------------------------\n");
		int cntr = 0;
		do
		{
			RPM->Read( (_QWORD)pElement, &iterator, sizeof(fb::EntityIterator<_QWORD>) );
			
			_QWORD Entity = (_QWORD)(pElement) - 0x50;
			printf("[%u] Element = 0x%I64X\n", cntr, pElement);
			printf("[%u] Entity  = 0x%I64X\n", cntr, Entity);

			fb::LinearTransform mTransform = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
			fb__GameEntity__GetTransform( (void*)Entity, &mTransform );

			printf("[%u] Transform:\n", cntr );
			printf("\t{ %f, %f, %f }\n", mTransform.right.x, mTransform.right.y, mTransform.right.z );
			printf("\t{ %f, %f, %f }\n", mTransform.up.x, mTransform.up.y, mTransform.up.z );
			printf("\t{ %f, %f, %f }\n", mTransform.forward.x, mTransform.forward.y, mTransform.forward.z );
			printf("\t{ %f, %f, %f }\n", mTransform.trans.x, mTransform.trans.y, mTransform.trans.z );
			printf("----------------------------------------------\n");
			cntr++;

			pElement = iterator.m_pFirst.GetPtr();
		} while ( ValidPointer(pElement) );
	}
	
	system("pause");
	return ERROR_SUCCESS;
};

void* DecryptPointer( DWORD64 EncryptedPtr, DWORD64 PointerKey )
{
	_QWORD pObfuscationMgr = NULL;
	if ( !RPM->Read( OFFSET_ObfuscationMgr, &pObfuscationMgr, sizeof(_QWORD) ) || !ValidPointer(pObfuscationMgr))
		return nullptr;
 
	if ( !(EncryptedPtr & 0x8000000000000000) )
		return nullptr; //invalid ptr
 
	_QWORD pObfuscationMgr_0xE0 = NULL;
	if ( !RPM->Read( pObfuscationMgr + 0xE0, &pObfuscationMgr_0xE0, sizeof(_QWORD) ))
		return nullptr;

	_QWORD pObfuscationMgr_0x100 = NULL;
	if ( !RPM->Read( pObfuscationMgr + 0x100, &pObfuscationMgr_0x100, sizeof(_QWORD) ))
		return nullptr;

	_QWORD hashtableKey = pObfuscationMgr_0xE0 ^ PointerKey;
 
	hashtable<_QWORD>* table = (hashtable<_QWORD>*)( pObfuscationMgr + 0x78 );
	hashtable_iterator<_QWORD> iterator = {};
 
	
	if ( !hashtable_find( table, &iterator, hashtableKey ) ) 
		return nullptr;

	_QWORD EncryptionKey = NULL;
 
	_QWORD EncryptionKeyFnc = pObfuscationMgr_0xE0 ^ pObfuscationMgr_0x100;
	if ( EncryptionKeyFnc > 0x140000000 && EncryptionKeyFnc < 0x14FFFFFFF )
		EncryptionKey = (_QWORD)(iterator.mpNode->mValue.second) ^ (_QWORD)(0x598447EFD7A36912);
	else
		EncryptionKey = (_QWORD)DecodePointerExternal( RPM->GetProcessHandle(),(_QWORD)iterator.mpNode->mValue.second );
 
	EncryptionKey ^= (5 * EncryptionKey);
 
	_QWORD DecryptedPtr = NULL;
	BYTE* pDecryptedPtrBytes = (BYTE*)&DecryptedPtr;
	BYTE* pEncryptedPtrBytes = (BYTE*)&EncryptedPtr;
	BYTE* pKeyBytes = (BYTE*)&EncryptionKey;
 
	for (char i = 0; i < 7; i++)
	{
		pDecryptedPtrBytes[i] = ( pKeyBytes[i] * 0x3B ) ^ ( pEncryptedPtrBytes[i] + pKeyBytes[i] );
		EncryptionKey += 8;
	}
	pDecryptedPtrBytes[7] = pEncryptedPtrBytes[7];
 
	DecryptedPtr &= ~( 0x8000000000000000 ); //to exclude the check bit
 
	return (void*)DecryptedPtr;
}

void fb__GameEntity__GetTransform( void* _this, fb::LinearTransform* OutMatrix )
{
	
	DWORD_PTR m_collection = NULL;// = *(DWORD_PTR *)( (DWORD_PTR)_this  + 0x38);
	RPM->Read( ((DWORD_PTR)_this  + 0x38), &m_collection, sizeof(DWORD_PTR) );
	if (!ValidPointer(m_collection)) return;

	unsigned __int8 offsets[2];
	RPM->Read( (m_collection + 9), &offsets, sizeof(unsigned __int8) * 2 );

	unsigned __int8 _9  = offsets[0];// = *(unsigned __int8 *)(m_collection + 9);
	unsigned __int8 _10 = offsets[1];// = *(unsigned __int8 *)(m_collection + 10);

	DWORD_PTR ComponentCollectionOffset = 0x20 * (_10 + (2 * _9) );

	RPM->Read( (m_collection + ComponentCollectionOffset + 0x10), OutMatrix, sizeof(fb::LinearTransform) );
}

bool __fastcall hashtable_find(hashtable<_QWORD>* table, hashtable_iterator<_QWORD>* iterator, _QWORD key)
{  
	unsigned int mnBucketCount = NULL;//;
	if ( !RPM->Read( (_QWORD)&table->mnBucketCount, &mnBucketCount, sizeof(unsigned int) ))
		return false;

	//bfv
	unsigned int startCount = (_QWORD)(key) % (_QWORD)(mnBucketCount);
 
	//bf1
	//unsigned int startCount = (unsigned int)(key) % mnBucketCount;

	static hash_node<_QWORD> node = {};
	void *mpBucketArray = NULL;
	if ( !RPM->Read( (_QWORD)&table->mpBucketArray, &mpBucketArray, sizeof(void*) ))
		return false;

	void* pNode = NULL;
	if ( !RPM->Read( (_QWORD)mpBucketArray + 8 * startCount, &pNode, sizeof(void*) ))
		return false;

	if ( ValidPointer(pNode) ) 
	{
		if ( !RPM->Read( (_QWORD)pNode, &node, sizeof(hash_node<_QWORD>) ))
		return false;
		if (!node.mValue.first)
			goto LABEL_4;
		while ( key != node.mValue.first )
		{
			if ( !ValidPointer(node.mpNext) || !RPM->Read( (_QWORD)node.mpNext, &node, sizeof(hash_node<_QWORD>) ))
				goto LABEL_4;
		}
		iterator->mpNode = &node;
		return true;
	}
	else
	{
LABEL_4:
		return false;
	}
	return false;
}
