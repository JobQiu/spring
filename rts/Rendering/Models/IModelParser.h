/* This file is part of the Spring engine (GPL v2 or later), see LICENSE.html */

#ifndef IMODELPARSER_H
#define IMODELPARSER_H

#include <unordered_map>
#include <deque>

#include <string>

#include "System/Threading/SpringMutex.h"

namespace boost {
	class thread;
};

struct S3DModel;
struct S3DModelPiece;

class IModelParser
{
public:
	virtual ~IModelParser() {}
	virtual S3DModel* Load(const std::string& name) = 0;
};


struct LoadQueue {
public:
	LoadQueue(): thread(nullptr) {}
	~LoadQueue() { Join(); }

	void Pump();
	void Push(const std::string& modelName);

	void GrabLock() { mutex.lock(); }
	void FreeLock() { mutex.unlock(); }

	void Join();
private:
	std::deque<std::string> queue;
	spring::mutex mutex;
	boost::thread* thread;
};



class CModelLoader
{
public:
	static CModelLoader& GetInstance();

	void Init();
	void Kill();

	S3DModel* LoadModel(std::string name, bool preload = false);

	std::string FindModelPath(std::string name) const;

	bool IsValid() const { return (!formats.empty()); }
	void PreloadModel(const std::string& name) { assert(IsValid()); loadQueue.Push(name); }

public:
	typedef std::unordered_map<std::string, unsigned int> ModelMap; // "armflash.3do" --> id
	typedef std::unordered_map<std::string, unsigned int> FormatMap; // "3do" --> MODELTYPE_3DO
	typedef std::unordered_map<unsigned int, IModelParser*> ParserMap; // MODELTYPE_3DO --> parser

private:
	S3DModel* LoadCachedModel(const std::string& name, bool preload);
	S3DModel* CreateModel(const std::string& name, const std::string& path, bool preload);
	S3DModel* ParseModel(const std::string& name, const std::string& path);

	IModelParser* GetFormatParser(const std::string& pathExt);

	void KillModels();
	void KillParsers();

	void AddModelToCache(S3DModel* model, const std::string& name, const std::string& path);

	void CreateLists(S3DModel* o);
	void CreateListsNow(S3DModelPiece* o);

private:
	ModelMap cache;
	FormatMap formats;
	ParserMap parsers;
	// preloading
	LoadQueue loadQueue;

	// all unique models loaded so far
	std::vector<S3DModel*> models;
};

#define modelLoader (CModelLoader::GetInstance())

#endif /* IMODELPARSER_H */
