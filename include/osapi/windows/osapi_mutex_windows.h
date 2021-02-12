#ifndef OSAPI_MUTEX_WINDOWS_H
#define OSAPI_MUTEX_WINDOWS_H


class Mutex : public MutexInterface
{
private:
	HANDLE mHandle;

public:
	Mutex();

	virtual ~Mutex();

	virtual bool lock(unsigned int timeout);
	virtual void unlock();
};



#endif // OSAPI_MUTEX_WINDOWS_H
