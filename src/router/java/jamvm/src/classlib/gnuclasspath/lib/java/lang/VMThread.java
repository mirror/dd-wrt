/* VMThread -- VM interface for Thread of executable code
   Copyright (C) 2003 Free Software Foundation

This file is part of GNU Classpath.

GNU Classpath is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU Classpath is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Classpath; see the file COPYING.  If not, write to the
Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA.

Linking this library statically or dynamically with other modules is
making a combined work based on this library.  Thus, the terms and
conditions of the GNU General Public License cover the whole
combination.

As a special exception, the copyright holders of this library give you
permission to link this library with independent modules to produce an
executable, regardless of the license terms of these independent
modules, and to copy and distribute the resulting executable under
terms of your choice, provided that you also meet, for each linked
independent module, the terms and conditions of the license of that
module.  An independent module is a module which is not derived from
or based on this library.  If you modify this library, you may extend
this exception to your version of the library, but you are not
obligated to do so.  If you do not wish to do so, delete this
exception statement from your version. */

package java.lang;

/**
 * VM interface for Thread of executable code. Holds VM dependent state.
 * It is deliberately package local and final and should only be accessed
 * by the Thread class.
 * <p>
 * This is the GNU Classpath reference implementation, it should be adapted
 * for a specific VM.
 * <p>
 * The following methods must be implemented:
 * <ul>
 * <li>native void start(long stacksize);
 * <li>native void interrupt();
 * <li>native boolean isInterrupted();
 * <li>native void suspend();
 * <li>native void resume();
 * <li>native void nativeSetPriority(int priority);
 * <li>native void nativeStop(Throwable t);
 * <li>native static Thread currentThread();
 * <li>static native void yield();
 * <li>static native void sleep(long ms, int ns) throws InterruptedException;
 * <li>static native boolean interrupted();
 * <li>static native boolean holdsLock(Object obj);
 * </ul>
 * All other methods may be implemented to make Thread handling more efficient
 * or to implement some optional (and sometimes deprecated) behaviour. Default
 * implementations are provided but it is highly recommended to optimize them
 * for a specific VM.
 * 
 * @author Jeroen Frijters (jeroen@frijters.net)
 */
final class VMThread
{
    private long vmData;

    /**
     * The Thread object that this VM state belongs to.
     * Used in currentThread() and start().
     * Note: when this thread dies, this reference is *not* cleared
     */
    volatile Thread thread;

    /**
     * Creates a native Thread. This is called from the start method of Thread.
     * The Thread is started.
     *
     * @param thread The newly created Thread object
     * @param stacksize Indicates the requested stacksize. Normally zero,
     * non-zero values indicate requested stack size in bytes but it is up
     * to the specific VM implementation to interpret them and may be ignored.
     */
    static native void create(Thread thread, long stacksize);

    /**
     * Gets the name of the thread. Usually this is the name field of the
     * associated Thread object, but some implementation might choose to
     * return the name of the underlying platform thread.
     */
    String getName()
    {
	return thread.name;
    }

    /**
     * Set the name of the thread. Usually this sets the name field of the
     * associated Thread object, but some implementations might choose to
     * set the name of the underlying platform thread.
     * @param name The new name
     */
    void setName(String name)
    {
	thread.name = name;
    }

    /**
     * Set the thread priority field in the associated Thread object and
     * calls the native method to set the priority of the underlying
     * platform thread.
     * @param priority The new priority
     */
    void setPriority(int priority)
    {
	thread.priority = priority;
	nativeSetPriority(priority);
    }

    /**
     * Returns the priority. Usually this is the priority field from the
     * associated Thread object, but some implementation might choose to
     * return the priority of the underlying platform thread.
     * @return this Thread's priority
     */
    int getPriority()
    {
        return thread.priority;
    }

    /**
     * Returns true if the thread is a daemon thread. Usually this is the
     * daemon field from the associated Thread object, but some
     * implementation might choose to return the daemon state of the underlying
     * platform thread.
     * @return whether this is a daemon Thread or not
     */
    boolean isDaemon()
    {
        return thread.daemon;
    }

    /**
     * Returns the number of stack frames in this Thread.
     * Will only be called when when a previous call to suspend() returned true.
     *
     * @deprecated unsafe operation
     */
    int countStackFrames()
    {
	return 0;
    }

    /**
     * Wait the specified amount of time for the Thread in question to die.
     *
     * <p>Note that 1,000,000 nanoseconds == 1 millisecond, but most VMs do
     * not offer that fine a grain of timing resolution. Besides, there is
     * no guarantee that this thread can start up immediately when time expires,
     * because some other thread may be active.  So don't expect real-time
     * performance.
     *
     * @param ms the number of milliseconds to wait, or 0 for forever
     * @param ns the number of extra nanoseconds to sleep (0-999999)
     * @throws InterruptedException if the Thread is interrupted; it's
     *         <i>interrupted status</i> will be cleared
     */
    void join(long ms, int ns) throws InterruptedException {
        synchronized(thread) {
            if(thread.isAlive())
                thread.wait(ms, ns);
        }
    }

    /**
     * Cause this Thread to stop abnormally and throw the specified exception.
     * If you stop a Thread that has not yet started, the stop is ignored
     * (contrary to what the JDK documentation says).
     * <b>WARNING</b>This bypasses Java security, and can throw a checked
     * exception which the call stack is unprepared to handle. Do not abuse 
     * this power.
     *
     * <p>This is inherently unsafe, as it can interrupt synchronized blocks and
     * leave data in bad states.
     *
     * <p><b>NOTE</b> stop() should take care not to stop a thread if it is
     * executing code in this class.
     *
     * @param t the Throwable to throw when the Thread dies
     * @deprecated unsafe operation, try not to use
     */
    void stop(Throwable t) {}

    /**
     * Create a native thread on the underlying platform and start it executing
     * on the run method of this object.
     * @param stacksize the requested size of the native thread stack
     */
    native void start(long stacksize);

    /**
     * Interrupt this thread.
     */
    native void interrupt();

    /**
     * Determine whether this Thread has been interrupted, but leave
     * the <i>interrupted status</i> alone in the process.
     *
     * @return whether the Thread has been interrupted
     */
    native boolean isInterrupted();

    /**
     * Suspend this Thread.  It will not come back, ever, unless it is resumed.
     */
    void suspend() {}

    /**
     * Resume this Thread.  If the thread is not suspended, this method does
     * nothing.
     */
    void resume() {}

    /**
     * Set the priority of the underlying platform thread.
     *
     * @param priority the new priority
     */
    native void nativeSetPriority(int priority);

    /**
     * Return the Thread object associated with the currently executing
     * thread.
     *
     * @return the currently executing Thread
     */
    native static Thread currentThread();

    /**
     * Yield to another thread. The Thread will not lose any locks it holds
     * during this time. There are no guarantees which thread will be
     * next to run, and it could even be this one, but most VMs will choose
     * the highest priority thread that has been waiting longest.
     */
    static native void yield();

    /**
     * Suspend the current Thread's execution for the specified amount of
     * time. The Thread will not lose any locks it has during this time. There
     * are no guarantees which thread will be next to run, but most VMs will
     * choose the highest priority thread that has been waiting longest.
     *
     * <p>Note that 1,000,000 nanoseconds == 1 millisecond, but most VMs do
     * not offer that fine a grain of timing resolution. Besides, there is
     * no guarantee that this thread can start up immediately when time expires,
     * because some other thread may be active.  So don't expect real-time
     * performance.
     *
     * @param ms the number of milliseconds to sleep, or 0 for forever
     * @param ns the number of extra nanoseconds to sleep (0-999999)
     * @throws InterruptedException if the Thread is interrupted; it's
     *         <i>interrupted status</i> will be cleared
     * @throws IllegalArgumentException if ns is invalid
     */
    static native void sleep(long ms, int ns) throws InterruptedException;

    /**
     * Determine whether the current Thread has been interrupted, and clear
     * the <i>interrupted status</i> in the process.
     *
     * @return whether the current Thread has been interrupted
     */
    static native boolean interrupted();

    /**
     * Checks whether the current thread holds the monitor on a given object.
     * This allows you to do <code>assert Thread.holdsLock(obj)</code>.
     *
     * @param obj the object to check
     * @return true if the current thread is currently synchronized on obj
     * @throws NullPointerException if obj is null
     */
    static native boolean holdsLock(Object obj);


    /**
     * Returns the current state of the thread.
     * The value must be one of "BLOCKED", "NEW",
     * "RUNNABLE", "TERMINATED", "TIMED_WAITING" or
     * "WAITING".
     *
     * @return a string corresponding to one of the 
     *         thread enumeration states specified above.
     */
    native String getState();
}
