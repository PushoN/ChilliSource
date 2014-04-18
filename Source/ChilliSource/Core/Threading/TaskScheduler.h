//
//  TaskScheduler.h
//  Chilli Source
//  Created by Scott Downie on 09/08/2011.
//
//  The MIT License (MIT)
//
//  Copyright (c) 2011 Tag Games Limited
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.
//

#ifndef _CHILLISOURCE_CORE_THREADING_TASKSCHEDULER_H_
#define _CHILLISOURCE_CORE_THREADING_TASKSCHEDULER_H_

#include <ChilliSource/ChilliSource.h>
#include <ChilliSource/Core/System/AppSystem.h>
#include <ChilliSource/Core/Threading/ThreadPool.h>
#include <ChilliSource/Core/Threading/Task.h>

namespace ChilliSource
{
    namespace Core
    {
		//-------------------------------------------------
		/// System for scheduling tasks to be executed
		/// asynchronously by the thread pool or on
		/// the main thread.
		///
		/// @author S Downie
		//-------------------------------------------------
        class TaskScheduler : public AppSystem
        {
        public:
        
			CS_DECLARE_NAMEDTYPE(TaskScheduler);

            typedef std::function<void()> GenericTaskType;

			//------------------------------------------------
			/// @author S Downie
			///
			/// @param Interface Id
			///
			/// @return Whether this object is of the given type
			//------------------------------------------------
			bool IsA(InterfaceIDType in_interfaceId) const override;
            //------------------------------------------------
            /// The task will be placed into the
            /// task queue and be performed when a thread
            /// becomes available.
			///
			/// @author S Downie
            ///
            /// @param Task
            //------------------------------------------------
			void ScheduleTask(const GenericTaskType& in_task);
            //----------------------------------------------------
            /// Schedule a task to be executed by the main
            /// thread
			///
			/// @author S Downie
			///
			/// @param Task
            //----------------------------------------------------
			void ScheduleMainThreadTask(const GenericTaskType& insTask);
            //----------------------------------------------------
            /// Execute any tasks that have been scehduled
            /// for the main thread
			///
			/// @author S Downie
            //----------------------------------------------------
            void ExecuteMainThreadTasks();

		private:
			friend class Application;
			//-------------------------------------------------
			/// Factory create method called by application
			///
			/// @param Number of threads to create in pool
			///
			/// @return Ownership of new instance
			//-------------------------------------------------
			static TaskSchedulerUPtr Create(u32 in_numThreads);
			//-------------------------------------------------
			/// Private constructor to enforce use of create
			/// method
			///
			/// @param Number of threads to create in pool
			//-------------------------------------------------
			TaskScheduler(u32 in_numThreads);
			//-------------------------------------------------
			/// Called when the system is created. Creates
			/// the thread pool based on the set number of
			/// threads.
			///
			/// @author S Downie
			//-------------------------------------------------
			void OnInit() override;
			//-------------------------------------------------
			/// Called when the system is destroyed. Joins
			/// all the threads back to main.
			///
			/// @author S Downie
			//-------------------------------------------------
			void OnDestroy() override;
            
        private:
        
			u32 m_numThreads;
            ThreadPoolUPtr m_threadPool;
            
            std::recursive_mutex m_mainThreadQueueMutex;
            std::vector<GenericTaskType> m_mainThreadTasks;
        };
    }
}

#endif
