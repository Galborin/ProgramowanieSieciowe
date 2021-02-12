#ifndef OSAPI_MORTAL_THREAD_H
#define OSAPI_MORTAL_THREAD_H
 
 
class MortalThread : public Thread
{
  public:
    MortalThread(int priority, unsigned int stackSize, const char* name = "unnamed");
 
    virtual ~MortalThread();            
                        
    /** Sends termination signal to the thread. */
    void kill();
                        
  private:
    /** Implementation of the body method */
    virtual void body(void);
	
	protected:
		virtual void begin() = 0;
		virtual void loop() = 0;
		virtual void end() = 0;
	
	private:
		bool mKilled;
};
 
#endif // OSAPI_MORTAL_THREAD_H
