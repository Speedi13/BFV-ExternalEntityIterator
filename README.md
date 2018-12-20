# BFV-ExternalEntityIterator
This tool helps to search for iterable entities in BFV.<br />
All you have to do is change the ClassInfo address press recompile and restart the program.<br />
Why this tool? Starting in Battlefield 1 the game developer encrypted the entities linked list.<br />
This tool uses ReadProcessMemory to access the game's memory that's why its called external.<br />

If you want to learn more about the encryption used and how to decrypt it look at my other project.<br />
https://github.com/Speedi13/BFV-Decryption

## Console output
Iterating over all ClientSoldierEntity entities
![Demo pic](https://raw.githubusercontent.com/Speedi13/BFV-ExternalEntityIterator/master/ConsoleOutputScreenshot.png.png)
<br>

## Main code
```cpp
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
```
```cpp
////////////////////////////////////////////////////////////////////
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
		printf("\t{ %f, %f, %f }\n", mTransform.right.x,    mTransform.right.y,   mTransform.right.z );
		printf("\t{ %f, %f, %f }\n", mTransform.up.x,       mTransform.up.y,      mTransform.up.z );
		printf("\t{ %f, %f, %f }\n", mTransform.forward.x,  mTransform.forward.y, mTransform.forward.z );
		printf("\t{ %f, %f, %f }\n", mTransform.trans.x,    mTransform.trans.y,   mTransform.trans.z );
		printf("----------------------------------------------\n");
		cntr++;

		pElement = iterator.m_pFirst.GetPtr();
	} while ( ValidPointer(pElement) );
}
```
