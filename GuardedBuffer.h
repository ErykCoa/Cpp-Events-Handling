#ifndef GUARDEDBUFFER
#define GUARDEDBUFFER

#include <mutex>
#include <functional>


template<class DataStructure>
class GuardedBuffer
{
public:
	void push_back(typename DataStructure::value_type Data);
	void push(typename DataStructure::value_type Data);

	DataStructure PopBuffer();

	typename DataStructure::value_type PopOne();
	typename DataStructure::value_type SecurePopOne( bool & HasSucceeded);

	bool empty();
	unsigned size();

	using value_type = typename DataStructure;

private:
	DataStructure Buffer;
	std::mutex Lock;
};


template<class DataStructure>
inline void GuardedBuffer<DataStructure>::push_back(typename DataStructure::value_type Data)
{
	std::lock_guard<std::mutex> _lock(Lock);

	Buffer.push_back(Data);

}

template<class DataStructure>
inline void GuardedBuffer<DataStructure>::push(typename DataStructure::value_type Data)
{
	std::lock_guard<std::mutex> _lock(Lock);

	Buffer.push(Data);
}

template<class DataStructure>
inline typename DataStructure::value_type GuardedBuffer<DataStructure>::PopOne()
{
	std::lock_guard<std::mutex> _lock(Lock);

	auto Res = Buffer.front();

	Buffer.pop();

	return Res;
}

template<class DataStructure>
inline typename DataStructure::value_type GuardedBuffer<DataStructure>::SecurePopOne(bool & HasSucceeded)
{
	std::lock_guard<std::mutex> _lock(Lock);
	if (!Buffer.empty())
	{
		HasSucceeded = true;

		auto Res = Buffer.front();

		Buffer.pop();

		return Res;
	}
	else
	{
		HasSucceeded = false;
		return typename DataStructure::value_type();
	}
	
}

template<class DataStructure>
inline DataStructure GuardedBuffer<DataStructure>::PopBuffer()
{
	std::lock_guard<std::mutex> _lock(Lock);

	DataStructure Res = Buffer;

	Buffer = DataStructure{};

	return Res;
}

template<class DataStructure>
inline bool GuardedBuffer<DataStructure>::empty()
{
	std::lock_guard<std::mutex> _lock(Lock);
	return Buffer.empty();
}

template<class DataStructure>
inline unsigned GuardedBuffer<DataStructure>::size()
{
	std::lock_guard<std::mutex> _lock(Lock);
	return Buffer.size();
}

#endif
