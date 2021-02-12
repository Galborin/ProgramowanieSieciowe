#ifndef OSAPI_RECURSIVE_MUTEX_WINDOWS_H
#define OSAPI_RECURSIVE_MUTEX_WINDOWS_H

class RecursiveMutex : public MutexInterface
{
private:
	HANDLE mHandle;
	
public:
	RecursiveMutex();
	virtual ~RecursiveMutex();
	virtual bool lock(unsigned int timeout);
	virtual void unlock();
};


#endif // OSAPI_RECURSIVE_MUTEX_WINDOWS_H
