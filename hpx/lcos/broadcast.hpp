//  Copyright (c) 2013-2014 Thomas Heller
//  Copyright (c) 2013-2014 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

/// \file broadcast.hpp

#if defined(DOXYGEN)
namespace hpx { namespace lcos
{
    /// \brief Perform a distributed broadcast operation
    ///
    /// The function hpx::lcos::broadcast performs a distributed broadcast
    /// operation resulting in action invocations on a given set
    /// of global identifiers. The action can be either a plain action (in
    /// which case the global identifiers have to refer to localities) or a
    /// component action (in which case the global identifiers have to refer
    /// to instances of a component type which exposes the action.
    ///
    /// The given action is invoked asynchronously on all given identifiers,
    /// and the arguments ArgN are passed along to those invocations.
    ///
    /// \param ids       [in] A list of global identifiers identifying the
    ///                  target objects for which the given action will be
    ///                  invoked.
    /// \param argN      [in] Any number of arbitrary arguments (passed by
    ///                  by const reference) which will be forwarded to the
    ///                  action invocation.
    ///
    /// \returns         This function returns a future representing the result
    ///                  of the overall reduction operation.
    ///
    /// \note            If decltype(Action(...)) is void, then the result of
    ///                  this function is future<void>.
    ///
    template <typename Action, typename ArgN, ...>
    hpx::future<std::vector<decltype(Action(hpx::id_type, ArgN, ...))> >
    broadcast(
        std::vector<hpx::id_type> const & ids
      , ArgN argN, ...);

    /// \brief Perform an asynchronous (fire&forget) distributed broadcast operation
    ///
    /// The function hpx::lcos::broadcast_apply performs an asynchronous
    /// (fire&forget) distributed broadcast operation resulting in action
    /// invocations on a given set of global identifiers. The action can be
    /// either a plain action (in which case the global identifiers have to
    /// refer to localities) or a component action (in which case the global
    /// identifiers have to refer to instances of a component type which
    /// exposes the action.
    ///
    /// The given action is invoked asynchronously on all given identifiers,
    /// and the arguments ArgN are passed along to those invocations.
    ///
    /// \param ids       [in] A list of global identifiers identifying the
    ///                  target objects for which the given action will be
    ///                  invoked.
    /// \param argN      [in] Any number of arbitrary arguments (passed by
    ///                  by const reference) which will be forwarded to the
    ///                  action invocation.
    ///
    template <typename Action, typename ArgN, ...>
    void
    broadcast_apply(
        std::vector<hpx::id_type> const & ids
      , ArgN argN, ...);

    /// \brief Perform a distributed broadcast operation
    ///
    /// The function hpx::lcos::broadcast_with_index performs a distributed broadcast
    /// operation resulting in action invocations on a given set
    /// of global identifiers. The action can be either a plain action (in
    /// which case the global identifiers have to refer to localities) or a
    /// component action (in which case the global identifiers have to refer
    /// to instances of a component type which exposes the action.
    ///
    /// The given action is invoked asynchronously on all given identifiers,
    /// and the arguments ArgN are passed along to those invocations.
    ///
    /// The function passes the index of the global identifier in the given
    /// list of identifiers as the last argument to the action.
    ///
    /// \param ids       [in] A list of global identifiers identifying the
    ///                  target objects for which the given action will be
    ///                  invoked.
    /// \param argN      [in] Any number of arbitrary arguments (passed by
    ///                  by const reference) which will be forwarded to the
    ///                  action invocation.
    ///
    /// \returns         This function returns a future representing the result
    ///                  of the overall reduction operation.
    ///
    /// \note            If decltype(Action(...)) is void, then the result of
    ///                  this function is future<void>.
    ///
    template <typename Action, typename ArgN, ...>
    hpx::future<std::vector<decltype(Action(hpx::id_type, ArgN, ..., std::size_t))> >
    broadcast_with_index(
        std::vector<hpx::id_type> const & ids
      , ArgN argN, ...);

    /// \brief Perform an asynchronous (fire&forget) distributed broadcast operation
    ///
    /// The function hpx::lcos::broadcast_apply_with_index performs an asynchronous
    /// (fire&forget) distributed broadcast operation resulting in action
    /// invocations on a given set of global identifiers. The action can be
    /// either a plain action (in which case the global identifiers have to
    /// refer to localities) or a component action (in which case the global
    /// identifiers have to refer to instances of a component type which
    /// exposes the action.
    ///
    /// The given action is invoked asynchronously on all given identifiers,
    /// and the arguments ArgN are passed along to those invocations.
    ///
    /// The function passes the index of the global identifier in the given
    /// list of identifiers as the last argument to the action.
    ///
    /// \param ids       [in] A list of global identifiers identifying the
    ///                  target objects for which the given action will be
    ///                  invoked.
    /// \param argN      [in] Any number of arbitrary arguments (passed by
    ///                  by const reference) which will be forwarded to the
    ///                  action invocation.
    ///
    template <typename Action, typename ArgN, ...>
    void
    broadcast_apply_with_index(
        std::vector<hpx::id_type> const & ids
      , ArgN argN, ...);
}}
#else

#ifndef HPX_LCOS_BROADCAST_HPP
#define HPX_LCOS_BROADCAST_HPP

#include <hpx/hpx_fwd.hpp>
#include <hpx/lcos/future.hpp>
#include <hpx/lcos/when_all.hpp>
#include <hpx/lcos/async_colocated.hpp>
#include <hpx/runtime/applier/apply_colocated.hpp>
#include <hpx/runtime/actions/plain_action.hpp>
#include <hpx/runtime/naming/name.hpp>
#include <hpx/util/calculate_fanout.hpp>
#include <hpx/util/tuple.hpp>
#include <hpx/util/detail/count_num_args.hpp>
#include <hpx/util/detail/pack.hpp>

#include <boost/preprocessor/cat.hpp>
#include <boost/serialization/vector.hpp>

#include <vector>

#if !defined(HPX_BROADCAST_FANOUT)
#define HPX_BROADCAST_FANOUT 16
#endif

namespace hpx { namespace lcos
{
    namespace detail
    {
        ///////////////////////////////////////////////////////////////////////
        template <typename Action>
        struct broadcast_with_index
        {
            typedef typename Action::arguments_type arguments_type;
        };

        ///////////////////////////////////////////////////////////////////////
        template <typename Action>
        struct broadcast_result
        {
            typedef
                typename traits::promise_local_result<
                    typename hpx::actions::extract_action<
                        Action
                    >::remote_result_type
                >::type
                action_result;
            typedef
                typename boost::mpl::if_<
                    boost::is_same<void, action_result>
                  , void
                  , std::vector<action_result>
                >::type
                type;
        };

        template <typename Action>
        struct broadcast_result<broadcast_with_index<Action> >
          : broadcast_result<Action>
        {};

        ///////////////////////////////////////////////////////////////////////
        template <
            typename Action
          , typename ...Ts
        >
        //hpx::future<typename broadcast_result<Action>::type>
        typename broadcast_result<Action>::type
        broadcast_impl(
            Action const & act
          , std::vector<hpx::id_type> const & ids
          , std::size_t global_idx
          , boost::mpl::false_
          , Ts const&... vs
        );

        template <
            typename Action
          , typename ...Ts
        >
        //hpx::future<void>
        void
        broadcast_impl(
            Action const & act
          , std::vector<hpx::id_type> const & ids
          , std::size_t global_idx
          , boost::mpl::true_
          , Ts const&... vs
        );

        template <
            typename Action
          , typename ...Ts
        >
        void
        broadcast_apply_impl(
            Action const & act
          , std::vector<hpx::id_type> const & ids
          , std::size_t global_idx
          , Ts const&... vs
        );

        ///////////////////////////////////////////////////////////////////////
        template <
            typename Action
          , typename Futures
          , typename ...Ts
        >
        void
        broadcast_invoke(Action act, Futures& futures, hpx::id_type const& id
          , std::size_t
          , Ts const&... vs)
        {
            futures.push_back(
                hpx::async(
                    act
                  , id
                  , vs...
                )
            );
        }

        template <
            typename Action
          , typename Futures
          , typename ...Ts
        >
        void
        broadcast_invoke(broadcast_with_index<Action>, Futures& futures, hpx::id_type const& id
          , std::size_t global_idx
          , Ts const&... vs)
        {
            futures.push_back(
                hpx::async(
                    Action()
                  , id
                  , vs...
                  , global_idx
                )
            );
        }

        template <
            typename Action
          , typename Futures
          , typename Cont
          , typename ...Ts
        >
        void
        broadcast_invoke(Action act, Futures& futures, Cont && cont
          , hpx::id_type const& id
          , std::size_t
          , Ts const&... vs)
        {
            futures.push_back(
                hpx::async(
                    act
                  , id
                  , vs...
                ).then(std::forward<Cont>(cont))
            );
        }

        template <
            typename Action
          , typename Futures
          , typename Cont
          , typename ...Ts
        >
        void
        broadcast_invoke(broadcast_with_index<Action>, Futures& futures
          , Cont && cont
          , hpx::id_type const& id
          , std::size_t global_idx
          , Ts const&... vs)
        {
            futures.push_back(
                hpx::async(
                    Action()
                  , id
                  , vs...
                  , global_idx
                ).then(std::forward<Cont>(cont))
            );
        }

        template <
            typename Action
          , typename ...Ts
        >
        void
        broadcast_invoke_apply(Action act
          , hpx::id_type const& id
          , std::size_t
          , Ts const&... vs)
        {
            hpx::apply(
                act
              , id
              , vs...
            );
        }

        template <
            typename Action
          , typename ...Ts
        >
        void
        broadcast_invoke_apply(broadcast_with_index<Action>
          , hpx::id_type const& id
          , std::size_t global_idx
          , Ts const&... vs)
        {
            hpx::apply(
                Action()
              , id
              , vs...
              , global_idx
            );
        }

        ///////////////////////////////////////////////////////////////////////
        template <
            typename Action
          , typename IsVoid
          , typename ...Ts
        >
        struct broadcast_invoker
        {
            //static hpx::future<typename broadcast_result<Action>::type>
            static typename broadcast_result<Action>::type
            call(
                Action const & act
              , std::vector<hpx::id_type> const & ids
              , std::size_t global_idx
              , IsVoid
              , Ts const&... vs
            )
            {
                return
                    broadcast_impl(
                        act
                      , ids
                      , global_idx
                      , IsVoid()
                      , vs...
                    );
            }
        };

        template <
            typename Action
          , typename ...Ts
        >
        struct broadcast_apply_invoker
        {
            static void
            call(
                Action const & act
              , std::vector<hpx::id_type> const & ids
              , std::size_t global_idx
              , Ts const&... vs
            )
            {
                return
                    broadcast_apply_impl(
                        act
                      , ids
                      , global_idx
                      , vs...
                    );
            }
        };

        ///////////////////////////////////////////////////////////////////////
        template <typename Action, typename Is>
        struct make_broadcast_action_impl;

        template <typename Action, std::size_t ...Is>
        struct make_broadcast_action_impl<Action, util::detail::pack_c<std::size_t, Is...> >
        {
            typedef
                typename broadcast_result<Action>::action_result
                action_result;

            typedef detail::broadcast_invoker<
                        Action
                      , typename boost::is_same<void, action_result>::type
                      , typename util::tuple_element<
                            Is, typename Action::arguments_type
                        >::type...
                    >
                    broadcast_invoker_type;

            typedef
                typename HPX_MAKE_ACTION(broadcast_invoker_type::call)::type
                type;
        };

        template <typename Action>
        struct make_broadcast_action
          : make_broadcast_action_impl<
                Action
              , typename util::detail::make_index_pack<Action::arity>::type
            >
        {};

        template <typename Action>
        struct make_broadcast_action<broadcast_with_index<Action> >
          : make_broadcast_action_impl<
                broadcast_with_index<Action>
              , typename util::detail::make_index_pack<Action::arity - 1>::type
            >
        {};

        template <typename Action, typename Is>
        struct make_broadcast_apply_action_impl;

        template <typename Action, std::size_t ...Is>
        struct make_broadcast_apply_action_impl<Action, util::detail::pack_c<std::size_t, Is...> >
        {
            typedef
                typename broadcast_result<Action>::action_result
                action_result;

            typedef detail::broadcast_apply_invoker<
                        Action
                      , typename util::tuple_element<
                            Is, typename Action::arguments_type
                        >::type...
                    >
                    broadcast_invoker_type;

            typedef
                typename HPX_MAKE_ACTION(broadcast_invoker_type::call)::type
                type;
        };

        template <typename Action>
        struct make_broadcast_apply_action
          : make_broadcast_apply_action_impl<
                Action
              , typename util::detail::make_index_pack<Action::arity>::type
            >
        {};

        template <typename Action>
        struct make_broadcast_apply_action<broadcast_with_index<Action> >
          : make_broadcast_apply_action_impl<
                broadcast_with_index<Action>
              , typename util::detail::make_index_pack<Action::arity - 1>::type
            >
        {};

        ///////////////////////////////////////////////////////////////////////
        inline void return_void(
            hpx::future<std::vector<hpx::future<void> > >)
        {
            // todo: verify validity of all futures in the vector
        }

        template <
            typename Result
        >
        std::vector<Result>
        wrap_into_vector(hpx::future<Result> r)
        {
            return std::vector<Result>(1, r.get());
        }

        template <
            typename Result
        >
        std::vector<Result>
        return_result_type(
            hpx::future<std::vector<hpx::future<std::vector<Result> > > > r)
        {
            std::vector<Result> res;
            std::vector<hpx::future<std::vector<Result> > > fres = std::move(r.get());

            BOOST_FOREACH(hpx::future<std::vector<Result> >& f, fres)
            {
                std::vector<Result> t = std::move(f.get());
                res.reserve(res.capacity() + t.size());
                std::move(t.begin(), t.end(), std::back_inserter(res));
            }

            return std::move(res);
        }

        ///////////////////////////////////////////////////////////////////////
        template <
            typename Action
          , typename ...Ts
        >
        //hpx::future<void>
        void
        broadcast_impl(
            Action const & act
          , std::vector<hpx::id_type> const & ids
          , std::size_t global_idx
          , boost::mpl::true_
          , Ts const&... vs
        )
        {
            if(ids.empty()) return;// hpx::lcos::make_ready_future();

            std::size_t const local_fanout = HPX_BROADCAST_FANOUT;
            std::size_t local_size = (std::min)(ids.size(), local_fanout);
            std::size_t fanout = util::calculate_fanout(ids.size(), local_fanout);

            std::vector<hpx::future<void> > broadcast_futures;
            broadcast_futures.reserve(local_size + (ids.size()/fanout) + 1);
            for(std::size_t i = 0; i != local_size; ++i)
            {
                broadcast_invoke(
                    act
                  , broadcast_futures
                  , ids[i]
                  , global_idx + i
                  , vs...
                );
            }

            if(ids.size() > local_fanout)
            {
                std::size_t applied = local_fanout;
                std::vector<hpx::id_type>::const_iterator it =
                    ids.begin() + local_fanout;

                typedef
                    typename detail::make_broadcast_action<
                        Action
                    >::type
                    broadcast_impl_action;

                while(it != ids.end())
                {
                    HPX_ASSERT(ids.size() >= applied);

                    std::size_t next_fan = (std::min)(fanout, ids.size() - applied);
                    std::vector<hpx::id_type> ids_next(it, it + next_fan);

                    hpx::id_type id(ids_next[0]);
                    broadcast_futures.push_back(
                        hpx::async_colocated<broadcast_impl_action>(
                            id
                          , act
                          , std::move(ids_next)
                          , global_idx + applied
                          , boost::integral_constant<bool, true>::type()
                          , vs...
                        )
                    );

                    applied += next_fan;
                    it += next_fan;
                }
            }

            //return hpx::when_all(broadcast_futures).then(&return_void);
            hpx::when_all(broadcast_futures).then(&return_void).get();
        }

        template <
            typename Action
          , typename ...Ts
        >
        //hpx::future<typename broadcast_result<Action>::type>
        typename broadcast_result<Action>::type
        broadcast_impl(
            Action const & act
          , std::vector<hpx::id_type> const & ids
          , std::size_t global_idx
          , boost::mpl::false_
          , Ts const&... vs
        )
        {
            typedef
                typename broadcast_result<Action>::action_result
                action_result;
            typedef
                typename broadcast_result<Action>::type
                result_type;

            //if(ids.empty()) return hpx::lcos::make_ready_future(result_type());
            if(ids.empty()) return result_type();

            std::size_t const local_fanout = HPX_BROADCAST_FANOUT;
            std::size_t local_size = (std::min)(ids.size(), local_fanout);
            std::size_t fanout = util::calculate_fanout(ids.size(), local_fanout);

            std::vector<hpx::future<result_type> > broadcast_futures;
            broadcast_futures.reserve(local_size + (ids.size()/fanout) + 1);
            for(std::size_t i = 0; i != local_size; ++i)
            {
                broadcast_invoke(
                    act
                  , broadcast_futures
                  , &wrap_into_vector<action_result>
                  , ids[i]
                  , global_idx + i
                  , vs...
                );
            }

            if(ids.size() > local_fanout)
            {
                std::size_t applied = local_fanout;
                std::vector<hpx::id_type>::const_iterator it =
                    ids.begin() + local_fanout;

                typedef
                    typename detail::make_broadcast_action<
                        Action
                    >::type
                    broadcast_impl_action;

                while(it != ids.end())
                {
                    HPX_ASSERT(ids.size() >= applied);

                    std::size_t next_fan = (std::min)(fanout, ids.size() - applied);
                    std::vector<hpx::id_type> ids_next(it, it + next_fan);

                    hpx::id_type id(ids_next[0]);
                    broadcast_futures.push_back(
                        hpx::async_colocated<broadcast_impl_action>(
                            id
                          , act
                          , std::move(ids_next)
                          , global_idx + applied
                          , boost::integral_constant<bool, false>::type()
                          , vs...
                        )
                    );

                    applied += next_fan;
                    it += next_fan;
                }
            }

            return hpx::when_all(broadcast_futures).
                then(&return_result_type<action_result>).get();
        }

        ///////////////////////////////////////////////////////////////////////
        template <
            typename Action
          , typename ...Ts
        >
        void
        broadcast_apply_impl(
            Action const & act
          , std::vector<hpx::id_type> const & ids
          , std::size_t global_idx
          , Ts const&... vs
        )
        {
            if(ids.empty()) return;

            std::size_t const local_fanout = HPX_BROADCAST_FANOUT;
            std::size_t local_size = (std::min)(ids.size(), local_fanout);

            for(std::size_t i = 0; i != local_size; ++i)
            {
                broadcast_invoke_apply(
                    act
                  , ids[i]
                  , global_idx + i
                  , vs...
                );
            }

            if(ids.size() > local_fanout)
            {
                std::size_t applied = local_fanout;
                std::vector<hpx::id_type>::const_iterator it =
                    ids.begin() + local_fanout;

                typedef
                    typename detail::make_broadcast_apply_action<
                        Action
                    >::type
                    broadcast_impl_action;

                std::size_t fanout = util::calculate_fanout(ids.size(), local_fanout);
                while(it != ids.end())
                {
                    HPX_ASSERT(ids.size() >= applied);

                    std::size_t next_fan = (std::min)(fanout, ids.size() - applied);
                    std::vector<hpx::id_type> ids_next(it, it + next_fan);

                    hpx::id_type id(ids_next[0]);
                    hpx::apply_colocated<broadcast_impl_action>(
                        id
                      , act
                      , std::move(ids_next)
                      , global_idx + applied
                      , vs...
                    );

                    applied += next_fan;
                    it += next_fan;
                }
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    template <
        typename Action
      , typename ...Ts
    >
    hpx::future<
        typename detail::broadcast_result<Action>::type
    >
    broadcast(
        std::vector<hpx::id_type> const & ids
      , Ts const&... vs)
    {
        typedef
            typename detail::make_broadcast_action<Action>::type
            broadcast_impl_action;
        typedef
            typename detail::broadcast_result<Action>::action_result
            action_result;

        if (ids.empty())
        {
            typedef typename detail::broadcast_result<Action>::type
                result_type;

            return hpx::make_exceptional_future<result_type>(
                HPX_GET_EXCEPTION(bad_parameter,
                    "hpx::lcos::broadcast",
                    "empty list of targets for broadcast operation")
                );
        }

        return
            hpx::async_colocated<broadcast_impl_action>(
                ids[0]
              , Action()
              , ids
              , 0
              , typename boost::is_same<void, action_result>::type()
              , vs...
            );
    }

    template <
        typename Component, typename Signature, typename Derived
      , typename ...Ts
    >
    hpx::future<
        typename detail::broadcast_result<Derived>::type
    >
    broadcast(
        hpx::actions::basic_action<Component, Signature, Derived> /* act */
      , std::vector<hpx::id_type> const & ids
      , Ts const&... vs)
    {
        return broadcast<Derived>(
                ids
              , vs...
            );
    }

    ///////////////////////////////////////////////////////////////////////////
    template <
        typename Action
      , typename ...Ts
    >
    void
    broadcast_apply(
        std::vector<hpx::id_type> const & ids
      , Ts const&... vs)
    {
        typedef
            typename detail::make_broadcast_apply_action<Action>::type
            broadcast_impl_action;

        if (ids.empty())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "hpx::lcos::broadcast_apply",
                "empty list of of targets for broadcast operation");
            return;
        }

        hpx::apply_colocated<broadcast_impl_action>(
                ids[0]
              , Action()
              , ids
              , 0
              , vs...
            );
    }

    template <
        typename Component, typename Signature, typename Derived
      , typename ...Ts
    >
    void
    broadcast_apply(
        hpx::actions::basic_action<Component, Signature, Derived> /* act */
      , std::vector<hpx::id_type> const & ids
      , Ts const&... vs)
    {
        broadcast_apply<Derived>(
            ids
          , vs...
        );
    }

    ///////////////////////////////////////////////////////////////////////////
    template <
        typename Action
      , typename ...Ts
    >
    hpx::future<
        typename detail::broadcast_result<Action>::type
    >
    broadcast_with_index(
        std::vector<hpx::id_type> const & ids
      , Ts const&... vs)
    {
        return broadcast<detail::broadcast_with_index<Action> >(
                ids
              , vs...
            );
    }

    template <
        typename Component, typename Signature, typename Derived
      , typename ...Ts
    >
    hpx::future<
        typename detail::broadcast_result<Derived>::type
    >
    broadcast_with_index(
        hpx::actions::basic_action<Component, Signature, Derived> /* act */
      , std::vector<hpx::id_type> const & ids
      , Ts const&... vs)
    {
        return broadcast<detail::broadcast_with_index<Derived> >(
                ids
              , vs...
            );
    }

    ///////////////////////////////////////////////////////////////////////////
    template <
        typename Action
      , typename ...Ts
    >
    void
    broadcast_apply_with_index(
        std::vector<hpx::id_type> const & ids
      , Ts const&... vs)
    {
        broadcast_apply<detail::broadcast_with_index<Action> >(
            ids
          , vs...
        );
    }

    template <
        typename Component, typename Signature, typename Derived
      , typename ...Ts
    >
    void
    broadcast_apply_with_index(
        hpx::actions::basic_action<Component, Signature, Derived> /* act */
      , std::vector<hpx::id_type> const & ids
      , Ts const&... vs)
    {
        broadcast_apply<detail::broadcast_with_index<Derived> >(
            ids
          , vs...
        );
    }
}}

///////////////////////////////////////////////////////////////////////////////
#define HPX_REGISTER_BROADCAST_APPLY_ACTION_DECLARATION(...)                  \
    HPX_REGISTER_BROADCAST_APPLY_ACTION_DECLARATION_(__VA_ARGS__)             \
/**/
#define HPX_REGISTER_BROADCAST_APPLY_ACTION_DECLARATION_(...)                 \
    HPX_UTIL_EXPAND_(BOOST_PP_CAT(                                            \
        HPX_REGISTER_BROADCAST_APPLY_ACTION_DECLARATION_,                     \
            HPX_UTIL_PP_NARG(__VA_ARGS__)                                     \
    )(__VA_ARGS__))                                                           \
/**/

#define HPX_REGISTER_BROADCAST_APPLY_ACTION_DECLARATION_1(Action)             \
    HPX_REGISTER_BROADCAST_APPLY_ACTION_DECLARATION_2(Action, Action)         \
/**/
#define HPX_REGISTER_BROADCAST_APPLY_ACTION_DECLARATION_2(Action, Name)       \
    HPX_REGISTER_ACTION_DECLARATION(                                          \
        ::hpx::lcos::detail::make_broadcast_apply_action<Action>::type        \
      , BOOST_PP_CAT(broadcast_apply_, Name)                                  \
    )                                                                         \
    HPX_REGISTER_APPLY_COLOCATED_DECLARATION(                                 \
        ::hpx::lcos::detail::make_broadcast_apply_action<Action>::type        \
      , BOOST_PP_CAT(apply_colocated_broadcast_, Name)                        \
    )                                                                         \
/**/

///////////////////////////////////////////////////////////////////////////////
#define HPX_REGISTER_BROADCAST_APPLY_ACTION(...)                              \
    HPX_REGISTER_BROADCAST_APPLY_ACTION_(__VA_ARGS__)                         \
/**/
#define HPX_REGISTER_BROADCAST_APPLY_ACTION_(...)                             \
    HPX_UTIL_EXPAND_(BOOST_PP_CAT(                                            \
        HPX_REGISTER_BROADCAST_APPLY_ACTION_, HPX_UTIL_PP_NARG(__VA_ARGS__)   \
    )(__VA_ARGS__))                                                           \
/**/

#define HPX_REGISTER_BROADCAST_APPLY_ACTION_1(Action)                         \
    HPX_REGISTER_BROADCAST_APPLY_ACTION_2(Action, Action)                     \
/**/
#define HPX_REGISTER_BROADCAST_APPLY_ACTION_2(Action, Name)                   \
    HPX_REGISTER_PLAIN_ACTION(                                                \
        ::hpx::lcos::detail::make_broadcast_apply_action<Action>::type        \
      , BOOST_PP_CAT(broadcast_apply_, Name)                                  \
    )                                                                         \
    HPX_REGISTER_APPLY_COLOCATED(                                             \
        ::hpx::lcos::detail::make_broadcast_apply_action<Action>::type        \
      , BOOST_PP_CAT(apply_colocated_broadcast_, Name)                        \
    )                                                                         \
/**/

///////////////////////////////////////////////////////////////////////////////
#define HPX_REGISTER_BROADCAST_APPLY_WITH_INDEX_ACTION_DECLARATION(...)       \
    HPX_REGISTER_BROADCAST_APPLY_WITH_INDEX_ACTION_DECLARATION_(__VA_ARGS__)  \
/**/
#define HPX_REGISTER_BROADCAST_APPLY_WITH_INDEX_ACTION_DECLARATION_(...)      \
    HPX_UTIL_EXPAND_(BOOST_PP_CAT(                                            \
        HPX_REGISTER_BROADCAST_APPLY_WITH_INDEX_ACTION_DECLARATION_,          \
            HPX_UTIL_PP_NARG(__VA_ARGS__)                                     \
    )(__VA_ARGS__))                                                           \
/**/

#define HPX_REGISTER_BROADCAST_APPLY_WITH_INDEX_ACTION_DECLARATION_1(Action)  \
    HPX_REGISTER_BROADCAST_APPLY_WITH_INDEX_ACTION_DECLARATION_2(             \
        Action, Action)                                                       \
/**/
#define HPX_REGISTER_BROADCAST_APPLY_WITH_INDEX_ACTION_DECLARATION_2(         \
        Action, Name)                                                         \
    HPX_REGISTER_ACTION_DECLARATION(                                          \
        ::hpx::lcos::detail::make_broadcast_apply_action<                     \
            ::hpx::lcos::detail::broadcast_with_index<Action>                 \
        >::type                                                               \
      , BOOST_PP_CAT(broadcast_apply_with_index_, Name)                       \
    )                                                                         \
    HPX_REGISTER_APPLY_COLOCATED_DECLARATION(                                 \
        ::hpx::lcos::detail::make_broadcast_apply_action<                     \
            ::hpx::lcos::detail::broadcast_with_index<Action>                 \
        >::type                                                               \
      , BOOST_PP_CAT(apply_colocated_broadcast_with_index_, Name)             \
    )                                                                         \
/**/

///////////////////////////////////////////////////////////////////////////////
#define HPX_REGISTER_BROADCAST_APPLY_WITH_INDEX_ACTION(...)                   \
    HPX_REGISTER_BROADCAST_APPLY_WITH_INDEX_ACTION_(__VA_ARGS__)              \
/**/
#define HPX_REGISTER_BROADCAST_APPLY_WITH_INDEX_ACTION_(...)                  \
    HPX_UTIL_EXPAND_(BOOST_PP_CAT(                                            \
        HPX_REGISTER_BROADCAST_APPLY_WITH_INDEX_ACTION_,                      \
            HPX_UTIL_PP_NARG(__VA_ARGS__)                                     \
    )(__VA_ARGS__))                                                           \
/**/

#define HPX_REGISTER_BROADCAST_APPLY_WITH_INDEX_ACTION_1(Action)              \
    HPX_REGISTER_BROADCAST_APPLY_WITH_INDEX_ACTION_2(Action, Action)          \
/**/
#define HPX_REGISTER_BROADCAST_APPLY_WITH_INDEX_ACTION_2(Action, Name)        \
    HPX_REGISTER_PLAIN_ACTION(                                                \
        ::hpx::lcos::detail::make_broadcast_apply_action<                     \
            ::hpx::lcos::detail::broadcast_with_index<Action>                 \
        >::type                                                               \
      , BOOST_PP_CAT(broadcast_apply_with_index_, Name)                       \
    )                                                                         \
    HPX_REGISTER_APPLY_COLOCATED(                                             \
        ::hpx::lcos::detail::make_broadcast_apply_action<                     \
            ::hpx::lcos::detail::broadcast_with_index<Action>                 \
        >::type                                                               \
      , BOOST_PP_CAT(apply_colocated_broadcast_with_index_, Name)             \
    )                                                                         \
/**/

///////////////////////////////////////////////////////////////////////////////
#define HPX_REGISTER_BROADCAST_ACTION_DECLARATION(...)                        \
    HPX_REGISTER_BROADCAST_ACTION_DECLARATION_(__VA_ARGS__)                   \
/**/
#define HPX_REGISTER_BROADCAST_ACTION_DECLARATION_(...)                       \
    HPX_UTIL_EXPAND_(BOOST_PP_CAT(                                            \
        HPX_REGISTER_BROADCAST_ACTION_DECLARATION_,                           \
            HPX_UTIL_PP_NARG(__VA_ARGS__)                                     \
    )(__VA_ARGS__))                                                           \
/**/

#define HPX_REGISTER_BROADCAST_ACTION_DECLARATION_1(Action)                   \
    HPX_REGISTER_BROADCAST_ACTION_DECLARATION_2(Action, Action)               \
/**/
#define HPX_REGISTER_BROADCAST_ACTION_DECLARATION_2(Action, Name)             \
    HPX_REGISTER_ACTION_DECLARATION(                                          \
        ::hpx::lcos::detail::make_broadcast_action<Action>::type              \
      , BOOST_PP_CAT(broadcast_, Name)                                        \
    )                                                                         \
    HPX_REGISTER_ASYNC_COLOCATED_DECLARATION(                                 \
        ::hpx::lcos::detail::make_broadcast_action<Action>::type              \
      , BOOST_PP_CAT(async_colocated_broadcast_, Name)                        \
    )                                                                         \
/**/

///////////////////////////////////////////////////////////////////////////////
#define HPX_REGISTER_BROADCAST_ACTION(...)                                    \
    HPX_REGISTER_BROADCAST_ACTION_(__VA_ARGS__)                               \
/**/
#define HPX_REGISTER_BROADCAST_ACTION_(...)                                   \
    HPX_UTIL_EXPAND_(BOOST_PP_CAT(                                            \
        HPX_REGISTER_BROADCAST_ACTION_, HPX_UTIL_PP_NARG(__VA_ARGS__)         \
    )(__VA_ARGS__))                                                           \
/**/

#define HPX_REGISTER_BROADCAST_ACTION_1(Action)                               \
    HPX_REGISTER_BROADCAST_ACTION_2(Action, Action)                           \
/**/
#define HPX_REGISTER_BROADCAST_ACTION_2(Action, Name)                         \
    HPX_REGISTER_PLAIN_ACTION(                                                \
        ::hpx::lcos::detail::make_broadcast_action<Action>::type              \
      , BOOST_PP_CAT(broadcast_, Name)                                        \
    )                                                                         \
    HPX_REGISTER_ASYNC_COLOCATED(                                             \
        ::hpx::lcos::detail::make_broadcast_action<Action>::type              \
      , BOOST_PP_CAT(async_colocated_broadcast_, Name)                        \
    )                                                                         \
/**/

///////////////////////////////////////////////////////////////////////////////
#define HPX_REGISTER_BROADCAST_WITH_INDEX_ACTION_DECLARATION(...)             \
    HPX_REGISTER_BROADCAST_WITH_INDEX_ACTION_DECLARATION_(__VA_ARGS__)        \
/**/
#define HPX_REGISTER_BROADCAST_WITH_INDEX_ACTION_DECLARATION_(...)            \
    HPX_UTIL_EXPAND_(BOOST_PP_CAT(                                            \
        HPX_REGISTER_BROADCAST_WITH_INDEX_ACTION_DECLARATION_,                \
            HPX_UTIL_PP_NARG(__VA_ARGS__)                                     \
    )(__VA_ARGS__))                                                           \
/**/

#define HPX_REGISTER_BROADCAST_WITH_INDEX_ACTION_DECLARATION_1(Action)        \
    HPX_REGISTER_BROADCAST_WITH_INDEX_ACTION_DECLARATION_2(Action, Action)    \
/**/
#define HPX_REGISTER_BROADCAST_WITH_INDEX_ACTION_DECLARATION_2(Action, Name)  \
    HPX_REGISTER_ACTION_DECLARATION(                                          \
        ::hpx::lcos::detail::make_broadcast_action<                           \
            ::hpx::lcos::detail::broadcast_with_index<Action>                 \
        >::type                                                               \
      , BOOST_PP_CAT(broadcast_with_index_, Name)                             \
    )                                                                         \
    HPX_REGISTER_ASYNC_COLOCATED_DECLARATION(                                 \
        ::hpx::lcos::detail::make_broadcast_action<                           \
            ::hpx::lcos::detail::broadcast_with_index<Action>                 \
        >::type                                                               \
      , BOOST_PP_CAT(async_colocated_broadcast_with_index_, Name)             \
    )                                                                         \
/**/

///////////////////////////////////////////////////////////////////////////////
#define HPX_REGISTER_BROADCAST_WITH_INDEX_ACTION(...)                         \
    HPX_REGISTER_BROADCAST_WITH_INDEX_ACTION_(__VA_ARGS__)                    \
/**/
#define HPX_REGISTER_BROADCAST_WITH_INDEX_ACTION_(...)                        \
    HPX_UTIL_EXPAND_(BOOST_PP_CAT(                                            \
        HPX_REGISTER_BROADCAST_WITH_INDEX_ACTION_,                            \
            HPX_UTIL_PP_NARG(__VA_ARGS__)                                     \
    )(__VA_ARGS__))                                                           \
/**/

#define HPX_REGISTER_BROADCAST_WITH_INDEX_ACTION_1(Action)                    \
    HPX_REGISTER_BROADCAST_WITH_INDEX_ACTION_2(Action, Action)                \
/**/
#define HPX_REGISTER_BROADCAST_WITH_INDEX_ACTION_2(Action, Name)              \
    HPX_REGISTER_PLAIN_ACTION(                                                \
        ::hpx::lcos::detail::make_broadcast_action<                           \
            ::hpx::lcos::detail::broadcast_with_index<Action>                 \
        >::type                                                               \
      , BOOST_PP_CAT(broadcast_with_index_, Name)                             \
    )                                                                         \
    HPX_REGISTER_ASYNC_COLOCATED(                                             \
        ::hpx::lcos::detail::make_broadcast_action<                           \
            ::hpx::lcos::detail::broadcast_with_index<Action>                 \
        >::type                                                               \
      , BOOST_PP_CAT(async_colocated_broadcast_with_index_, Name)             \
    )                                                                         \
/**/

#endif

#endif // DOXYGEN
