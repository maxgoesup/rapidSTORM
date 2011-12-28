#ifndef DSTORM_JOB_RUN_H
#define DSTORM_JOB_RUN_H

#include "Queue.h"
#include "ComputationThread.h"
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <dStorm/Engine.h>
#include <dStorm/output/Output.h>

namespace dStorm {
namespace job {

class Run
{
  public:
    typedef input::Source< output::LocalizedImage > Input;
    typedef output::Output Output;
    enum Result { Succeeded, Failed, Restart };

    Run( boost::recursive_mutex& mutex, frame_index first_image,
         Input& input, Output& output, int piston_count );
    Result run();
    ~Run();

    void restart();
    std::auto_ptr<EngineBlock> block();

  private:
    boost::recursive_mutex& mutex;
    Queue queue;
    boost::condition unblocked;
    boost::ptr_vector<ComputationThread> threads;
    bool restarted, blocked;
    class Block;

    Input& input;
    Output& output;
    int piston_count;
};

}
}

#endif
