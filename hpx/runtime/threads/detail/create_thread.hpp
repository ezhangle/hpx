//  Copyright (c) 2007-2013 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(HPX_RUNTIME_THREADS_DETAIL_CREATE_THREAD_JAN_13_2013_0439PM)
#define HPX_RUNTIME_THREADS_DETAIL_CREATE_THREAD_JAN_13_2013_0439PM

#include <hpx/hpx_fwd.hpp>
#include <hpx/exception.hpp>
#include <hpx/runtime/threads/thread_data.hpp>
#include <hpx/runtime/threads/thread_init_data.hpp>
#include <hpx/runtime/threads/policies/scheduler_base.hpp>
#include <hpx/util/logging.hpp>

namespace hpx { namespace threads { namespace detail
{
    inline void create_thread(
        policies::scheduler_base* scheduler, thread_init_data& data,
        threads::thread_id_type& id,
        thread_state_enum initial_state = pending,
        bool run_now = true, error_code& ec = throws)
    {
        // verify parameters
        switch (initial_state) {
        case pending:
        case suspended:
            break;

        default:
            {
                std::ostringstream strm;
                strm << "invalid initial state: "
                     << get_thread_state_name(initial_state);
                HPX_THROWS_IF(ec, bad_parameter,
                    "threads::detail::create_thread",
                    strm.str());
                return;
            }
        }

#ifdef HPX_THREAD_MAINTAIN_DESCRIPTION
        if (0 == data.description)
        {
            HPX_THROWS_IF(ec, bad_parameter,
                "threads::detail::create_thread", "description is NULL");
            return;
        }
#endif

        thread_self* self = get_self_ptr();

#ifdef HPX_THREAD_MAINTAIN_PARENT_REFERENCE
        if (0 == data.parent_id) {
            if (self)
            {
                data.parent_id = threads::get_self_id().get();
                data.parent_phase = self->get_thread_phase();
            }
        }
        if (0 == data.parent_locality_id)
            data.parent_locality_id = get_locality_id();
#endif

        if (0 == data.scheduler_base)
            data.scheduler_base = scheduler;

        // Pass critical priority from parent to child.
        if (self)
        {
            if (thread_priority_critical == threads::get_self_id()->get_priority())
                data.priority = thread_priority_critical;
        }

        // create the new thread
        scheduler->create_thread(data, &id, initial_state, run_now, ec,
            data.num_os_thread);

        LTM_(info) << "register_thread(" << id << "): initial_state("
                   << get_thread_state_name(initial_state) << "), "
                   << "run_now(" << (run_now ? "true" : "false")
#ifdef HPX_THREAD_MAINTAIN_DESCRIPTION
                   << "), description(" << data.description
#endif
                   << ")";

        // potentially wake up waiting thread
        scheduler->do_some_work(data.num_os_thread);
    }
}}}

#endif

