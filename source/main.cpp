#include <GarrysMod/Lua/Interface.h>
#include <iostream>
#include <steam/isteamclient.h>
#include <steam/isteamugc.h>
#include <steam/steam_gameserver.h>
#include <string>
#include <list>
#include <algorithm>

struct QueuedFile {
	uint64 id;
	int ref;
};

struct CSteamworks {
	CSteamworks(GarrysMod::Lua::ILuaBase* LUA) : LUA(LUA) {}

	STEAM_GAMESERVER_CALLBACK(CSteamworks, OnItemDownloaded, DownloadItemResult_t);

	void QueueDownload(uint64 id, int ref);

	std::list<QueuedFile> fileQueue;
	GarrysMod::Lua::ILuaBase* LUA;
};

void CSteamworks::QueueDownload(uint64 id, int ref) {
	fileQueue.push_back({ id, ref });
}

void CSteamworks::OnItemDownloaded(DownloadItemResult_t* item) {
	//We only care about garry's mod events. We can potentially get others. 
	if (item->m_unAppID != 4000) return;
	uint64 id = item->m_nPublishedFileId;

	//Search for first queued file we can find.
	auto element = std::find_if(std::begin(fileQueue), std::end(fileQueue), [&](const QueuedFile& file) {
		return file.id == id;
	});

	//If it's not in the queue, it's not one of ours
	if (element == fileQueue.end())
		return;

	char folderPath[256];
	uint64 sizeOnDisk;
	uint32 timestamp;

	bool result = item->m_eResult == EResult::k_EResultOK;

	//We're okay, now continue by checking ItemInstallInfo
	if (result) {
		result = SteamGameServerUGC()->GetItemInstallInfo(item->m_nPublishedFileId, &sizeOnDisk, folderPath, sizeof(folderPath), &timestamp);
	}

	//The function's getting called either way, just send nil if the result is bad.
	LUA->ReferencePush(element->ref);
	result ? LUA->PushString(folderPath) : LUA->PushNil();
	LUA->PCall(1, 0, 0);
	LUA->ReferenceFree(element->ref);

	//Delete the element from our queue. 
	fileQueue.erase(element);
}

CSteamworks* steamworks = nullptr;

LUA_FUNCTION_STATIC(DownloadUGC) {
	LUA->CheckString(1);
	LUA->CheckType(2, GarrysMod::Lua::Type::Function);

	const char* id = LUA->GetString(1);
	uint64 num = std::stoull(id);
	bool success = SteamGameServerUGC()->DownloadItem(num, false);

	if (!success) {
		LUA->Push(2);
		LUA->PushNil();
		LUA->PCall(1, 0, 0);
	}
	else {
		LUA->Push(2);
		steamworks->QueueDownload(num, LUA->ReferenceCreate());
	}

	LUA->PushBool(success);
	return 1;
}

GMOD_MODULE_OPEN()
{
	steamworks = new CSteamworks(LUA);
	LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
		LUA->PushString("steamworks");
		LUA->CreateTable();
			LUA->PushString("DownloadUGC");
			LUA->PushCFunction(DownloadUGC);
			LUA->SetTable(-3);
		LUA->SetTable(-3);
	LUA->Pop();

	return 0;
}

GMOD_MODULE_CLOSE()
{
	delete steamworks;
	steamworks = nullptr;

	return 0;
}