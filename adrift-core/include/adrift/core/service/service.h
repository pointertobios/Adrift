// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <concepts>
#include <memory>
#include <type_traits>

#include "adrift/assert.h"
#include "adrift/async/future.h"
#include "adrift/async/join_handle.h"
#include "adrift/container/hash_map.h"
#include "adrift/core/service/service_controller.h"
#include "adrift/types/string_id.h"
#include "adrift/util/type_mutate.h"

namespace adrift::core::service {

class Service;

template<typename S>
concept ServiceImpl = requires {
    std::derived_from<S, Service>;
    !std::is_same_v<S, Service>;
};

using ServiceCreator = std::tuple<type_id, std::unique_ptr<Service>> (*)();

class Service : public util::type_mutate<Service> {
public:
    template<typename S>
    static constexpr bool static_mutatable = ServiceImpl<S>;

    static ServiceCreator get_service_creator(string_id name);

    template<ServiceImpl S>
    static ServiceCreator register_creator() {
        auto ctor = +[] -> std::tuple<type_id, std::unique_ptr<Service>> {
            return {type_id::of<S>(), std::make_unique<S>()};
        };
        auto res = s_services_of_components.insert(string_id{std::meta::identifier_of(^^S)}, ctor);
        ADRIFT_ASSERT(res);
        return ctor;
    }

    Service(const Service &) = delete;
    Service &operator=(const Service &) = delete;

    Service(Service &&) = delete;
    Service &operator=(Service &&) = delete;

    virtual ustr name() const = 0;

    void start(ServiceController &service_ctrl);

    async::future<> join();

protected:
    constexpr Service(type_id type)
            : type_mutate<Service>{type} {}

    virtual async::future<> run(ServiceController &service_ctrl) = 0;

private:
    async::join_handle<> m_run_task{};

    inline static hash_map<string_id, ServiceCreator> s_services_of_components{};
};

#define adrift_core_service_register(service_impl)                              \
    namespace __registration_of_##service_impl {                                \
        inline ::adrift::core::service::ServiceCreator creator =                \
            ::adrift::core::service::Service::register_creator<service_impl>(); \
    }

};  // namespace adrift::core::service
