///////////////////////////////////////////////////////////////////////////////
// FILE:          TaskSet_CopyMemory.cpp
// PROJECT:       Micro-Manager
// SUBSYSTEM:     MMCore
//-----------------------------------------------------------------------------
// DESCRIPTION:   Task set for parallelized memory copy.
//
// AUTHOR:        Tomas Hanak, tomas.hanak@teledyne.com, 03/03/2021
//                Andrej Bencur, andrej.bencur@teledyne.com, 03/03/2021
//
// COPYRIGHT:     Teledyne Digital Imaging US, Inc., 2021
//
// LICENSE:       This file is distributed under the "Lesser GPL" (LGPL) license.
//                License text is included with the source distribution.
//
//                This file is distributed in the hope that it will be useful,
//                but WITHOUT ANY WARRANTY; without even the implied warranty
//                of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//                IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//                CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//                INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES.

#include "TaskSet_CopyMemory.h"

#include <algorithm>
#include <cassert>
#include <cstring>

TaskSet_CopyMemory::ATask::ATask(std::shared_ptr<Semaphore> semDone, size_t taskIndex, size_t totalTaskCount)
    : Task(semDone, taskIndex, totalTaskCount),// the usedTaskCount_ of task first is set to the same as totalTaskCount
    dst_(NULL),
    src_(NULL),
    bytes_(0)
{
}

void TaskSet_CopyMemory::ATask::SetUp(void* dst, const void* src, size_t bytes, size_t usedTaskCount)
{
    dst_ = dst;
    src_ = src;
    bytes_ = bytes;
    // Then the usedTaskCount_ of task is set to the size of task really need to use
    usedTaskCount_ = usedTaskCount;
}

void TaskSet_CopyMemory::ATask::Execute()
{
    if (taskIndex_ >= usedTaskCount_)
        return;

    size_t chunkBytes = bytes_ / usedTaskCount_;
    const size_t chunkOffset = taskIndex_ * chunkBytes;
    if (taskIndex_ == usedTaskCount_ - 1)
        chunkBytes += bytes_ % usedTaskCount_;// the last task is responsible for the all last Bytes

    void* dst = static_cast<char*>(dst_) + chunkOffset;// do not plus or minus on void pointer
    const void* src = static_cast<const char*>(src_) + chunkOffset;

    std::memcpy(dst, src, chunkBytes);
}

TaskSet_CopyMemory::TaskSet_CopyMemory(std::shared_ptr<ThreadPool> pool)
    : TaskSet(pool)
{
    CreateTasks<ATask>();// usedTaskCount_ first is set to the size of pool
}

void TaskSet_CopyMemory::SetUp(void* dst, const void* src, size_t bytes)
{
    assert(dst != NULL);
    assert(src != NULL);
    assert(bytes > 0);

    // Call memcpy directly without threading for small frames up to 1MB
    // Otherwise do parallel copy and add one thread for each 1MB
    // The limits were found experimentally
    // Then usedTaskCount_ is set to the count of tasks
    usedTaskCount_ = std::min<size_t>(1 + bytes / 1000000, tasks_.size());// (1 + bytes / 1000000) seems to be wrong
    // formula : size = (number + unit - 1) / unit. If the last task is responsible for the all last Bytes, the number of tasks should be size - 1.
    if (usedTaskCount_ == 1)
    {
        std::memcpy(dst, src, bytes);
        return;
    }

    for (Task* task : tasks_)
        static_cast<ATask*>(task)->SetUp(dst, src, bytes, usedTaskCount_);
}

void TaskSet_CopyMemory::Execute()
{
    if (usedTaskCount_ == 1)
        return; // Already done in SetUp, nothing to execute

    TaskSet::Execute();
}

void TaskSet_CopyMemory::Wait()
{
    if (usedTaskCount_ == 1)
        return; // Already done in SetUp, nothing to wait for

    semaphore_->Wait(usedTaskCount_);// Block this thread till all tasks have been done
}

void TaskSet_CopyMemory::MemCopy(void* dst, const void* src, size_t bytes)
{
    SetUp(dst, src, bytes);
    Execute();
    Wait();
}
