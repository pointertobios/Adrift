// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#include "adrift/core/ecs/manager.h"
#include "adrift/core/ecs/component.h"
#include "adrift/core/ecs/component_storage.h"
#include "adrift/core/ecs/entity.h"
#include "adrift/test/test.h"

#include <ranges>
#include <vector>

using namespace adrift;
using namespace adrift::core::ecs;

struct TestHealth : Component<TestHealth> {
    using Storage = DenseComponentStorage<TestHealth>;

    int hp;

    TestHealth(Entity e, ComponentID id, int hp)
            : Component<TestHealth>(e, id)
            , hp{hp} {}
};

struct TestMana : Component<TestMana> {
    using Storage = SparseComponentStorage<TestMana>;

    float amount;

    TestMana(Entity e, ComponentID id, float amount)
            : Component<TestMana>(e, id)
            , amount{amount} {}
};

ADRIFT_SYNC_TEST(manager_dense_create_and_get_component) {
    Manager<TestHealth> mgr;
    ComponentID id{1};
    Entity entity{100};

    auto &created = mgr.create(id, entity, id, 80);
    ADRIFT_SYNC_ASSERT(created.hp == 80, "created component should store the passed hp");
    ADRIFT_SYNC_ASSERT(created.owner_entity() == entity, "created component should store owner entity");

    auto &retrieved = mgr.get_component(id);
    ADRIFT_SYNC_ASSERT(&retrieved == &created, "get_component should return the same instance");
    ADRIFT_SYNC_ASSERT(retrieved.hp == 80, "retrieved component should have the same hp");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(manager_dense_create_multiple) {
    Manager<TestHealth> mgr;
    Entity e{1};

    mgr.create(ComponentID{1}, e, ComponentID{1}, 10);
    mgr.create(ComponentID{2}, e, ComponentID{2}, 20);
    mgr.create(ComponentID{3}, e, ComponentID{3}, 30);

    ADRIFT_SYNC_ASSERT(mgr.get_component(ComponentID{1}).hp == 10, "component 1 hp should be 10");
    ADRIFT_SYNC_ASSERT(mgr.get_component(ComponentID{2}).hp == 20, "component 2 hp should be 20");
    ADRIFT_SYNC_ASSERT(mgr.get_component(ComponentID{3}).hp == 30, "component 3 hp should be 30");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(manager_dense_destroy_and_recreate) {
    Manager<TestHealth> mgr;
    Entity e{1};

    mgr.create(ComponentID{1}, e, ComponentID{1}, 99);
    mgr.destroy(ComponentID{1});

    // Recreate with same id — slot should be reusable
    auto &recreated = mgr.create(ComponentID{1}, e, ComponentID{1}, 88);
    ADRIFT_SYNC_ASSERT(recreated.hp == 88, "recreated component after destroy should have new value");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(manager_dense_destroy_preserves_others) {
    Manager<TestHealth> mgr;
    Entity e{1};

    for (int i = 1; i <= 5; ++i) {
        mgr.create(ComponentID{static_cast<u64>(i)}, e, ComponentID{static_cast<u64>(i)}, i * 10);
    }

    mgr.destroy(ComponentID{3});

    ADRIFT_SYNC_ASSERT(mgr.get_component(ComponentID{1}).hp == 10, "component 1 should be intact");
    ADRIFT_SYNC_ASSERT(mgr.get_component(ComponentID{2}).hp == 20, "component 2 should be intact");
    ADRIFT_SYNC_ASSERT(mgr.get_component(ComponentID{4}).hp == 40, "component 4 should be intact");
    ADRIFT_SYNC_ASSERT(mgr.get_component(ComponentID{5}).hp == 50, "component 5 should be intact");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(manager_sparse_create_and_get_component) {
    Manager<TestMana> mgr;
    ComponentID id{1};
    Entity entity{50};

    auto &created = mgr.create(id, entity, id, 3.14f);
    ADRIFT_SYNC_ASSERT(created.amount == 3.14f, "sparse created component should store amount");
    ADRIFT_SYNC_ASSERT(created.owner_entity() == entity, "sparse component should store owner entity");

    auto &retrieved = mgr.get_component(id);
    ADRIFT_SYNC_ASSERT(&retrieved == &created, "sparse get_component should return same instance");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(manager_sparse_create_multiple) {
    Manager<TestMana> mgr;
    Entity e{1};

    mgr.create(ComponentID{1}, e, ComponentID{1}, 1.0f);
    mgr.create(ComponentID{2}, e, ComponentID{2}, 2.0f);
    mgr.create(ComponentID{3}, e, ComponentID{3}, 3.0f);

    ADRIFT_SYNC_ASSERT(mgr.get_component(ComponentID{1}).amount == 1.0f, "sparse component 1");
    ADRIFT_SYNC_ASSERT(mgr.get_component(ComponentID{2}).amount == 2.0f, "sparse component 2");
    ADRIFT_SYNC_ASSERT(mgr.get_component(ComponentID{3}).amount == 3.0f, "sparse component 3");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(manager_sparse_destroy) {
    Manager<TestMana> mgr;
    Entity e{1};

    mgr.create(ComponentID{1}, e, ComponentID{1}, 1.5f);
    mgr.create(ComponentID{2}, e, ComponentID{2}, 2.5f);

    mgr.destroy(ComponentID{1});

    // Remaining component should be intact
    ADRIFT_SYNC_ASSERT(mgr.get_component(ComponentID{2}).amount == 2.5f, "remaining sparse component intact");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(manager_dense_get_components_all) {
    Manager<TestHealth> mgr;
    Entity e{1};

    mgr.create(ComponentID{1}, e, ComponentID{1}, 10);
    mgr.create(ComponentID{2}, e, ComponentID{2}, 20);
    mgr.create(ComponentID{3}, e, ComponentID{3}, 30);

    int count = 0;
    int sum = 0;
    for (auto &c : mgr.get_components()) {
        sum += c.hp;
        count++;
    }
    ADRIFT_SYNC_ASSERT(count == 3, "get_components should yield 3 components");
    ADRIFT_SYNC_ASSERT(sum == 60, "sum of hp should be 60");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(manager_dense_get_components_all_const) {
    Manager<TestHealth> mgr;
    Entity e{1};

    mgr.create(ComponentID{1}, e, ComponentID{1}, 5);
    mgr.create(ComponentID{2}, e, ComponentID{2}, 15);

    const auto &cmgr = mgr;
    int count = 0;
    for (const auto &c : cmgr.get_components()) {
        count++;
        ADRIFT_SYNC_ASSERT(c.owner_entity() == e, "all const components should belong to the same entity");
    }
    ADRIFT_SYNC_ASSERT(count == 2, "const get_components should yield 2 components");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(manager_dense_get_components_by_entity) {
    Manager<TestHealth> mgr;
    Entity ea{1};
    Entity eb{2};

    mgr.create(ComponentID{1}, ea, ComponentID{1}, 100);
    mgr.create(ComponentID{2}, ea, ComponentID{2}, 200);
    mgr.create(ComponentID{3}, eb, ComponentID{3}, 300);

    int count_a = 0;
    for (auto &c : mgr.get_components(ea)) {
        ADRIFT_SYNC_ASSERT(c.owner_entity() == ea, "filtered components should belong to ea");
        count_a++;
    }
    ADRIFT_SYNC_ASSERT(count_a == 2, "ea should have 2 components");

    int count_b = 0;
    for (auto &c : mgr.get_components(eb)) {
        ADRIFT_SYNC_ASSERT(c.owner_entity() == eb, "filtered components should belong to eb");
        count_b++;
    }
    ADRIFT_SYNC_ASSERT(count_b == 1, "eb should have 1 component");

    // Unknown entity
    ADRIFT_SYNC_ASSERT(std::ranges::empty(mgr.get_components(Entity{99})), "unknown entity should be empty");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(manager_dense_get_components_by_entity_const) {
    Manager<TestHealth> mgr;
    Entity e1{10};
    Entity e2{20};

    mgr.create(ComponentID{1}, e1, ComponentID{1}, 1);
    mgr.create(ComponentID{2}, e2, ComponentID{2}, 2);

    const auto &cmgr = mgr;
    int count = 0;
    for (const auto &c : cmgr.get_components(e1)) {
        (void)c;
        count++;
    }
    ADRIFT_SYNC_ASSERT(count == 1, "const by-entity should yield 1 component");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(manager_sparse_get_components_all) {
    Manager<TestMana> mgr;
    Entity e{1};

    mgr.create(ComponentID{1}, e, ComponentID{1}, 1.0f);
    mgr.create(ComponentID{2}, e, ComponentID{2}, 2.0f);
    mgr.create(ComponentID{3}, e, ComponentID{3}, 3.0f);

    int count = 0;
    float sum = 0.0f;
    for (auto &c : mgr.get_components()) {
        sum += c.amount;
        count++;
    }
    ADRIFT_SYNC_ASSERT(count == 3, "sparse get_components should yield 3");
    ADRIFT_SYNC_ASSERT(sum == 6.0f, "sum should be 6.0");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(manager_sparse_get_components_by_entity) {
    Manager<TestMana> mgr;
    Entity e1{1};
    Entity e2{2};

    mgr.create(ComponentID{1}, e1, ComponentID{1}, 10.0f);
    mgr.create(ComponentID{2}, e2, ComponentID{2}, 20.0f);
    mgr.create(ComponentID{3}, e1, ComponentID{3}, 30.0f);

    int count1 = 0;
    for (auto &c : mgr.get_components(e1)) {
        ADRIFT_SYNC_ASSERT(c.owner_entity() == e1, "should belong to e1");
        count1++;
    }
    ADRIFT_SYNC_ASSERT(count1 == 2, "sparse e1 should have 2");

    int count2 = 0;
    for (auto &c : mgr.get_components(e2)) {
        (void)c;
        count2++;
    }
    ADRIFT_SYNC_ASSERT(count2 == 1, "sparse e2 should have 1");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(manager_dense_vget_components_all) {
    Manager<TestHealth> mgr;
    Entity e{1};

    mgr.create(ComponentID{1}, e, ComponentID{1}, 10);
    mgr.create(ComponentID{2}, e, ComponentID{2}, 20);
    mgr.create(ComponentID{3}, e, ComponentID{3}, 30);

    Manager<> &base = mgr;
    auto comps = base.vget_components();
    ADRIFT_SYNC_ASSERT(comps.size() == 3, "vget_components should return 3 refs");

    int sum = 0;
    for (auto &ref : comps) {
        auto &c = ref.get();
        sum += c.as<TestHealth>().hp;
    }
    ADRIFT_SYNC_ASSERT(sum == 60, "sum via vget_components should be 60");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(manager_dense_vget_components_all_const) {
    Manager<TestHealth> mgr;
    Entity e{1};

    mgr.create(ComponentID{1}, e, ComponentID{1}, 5);
    mgr.create(ComponentID{2}, e, ComponentID{2}, 15);

    const Manager<> &base = mgr;
    auto comps = base.vget_components();
    ADRIFT_SYNC_ASSERT(comps.size() == 2, "const vget_components should return 2 refs");

    int sum = 0;
    for (auto &ref : comps) {
        sum += ref.get().as<TestHealth>().hp;
    }
    ADRIFT_SYNC_ASSERT(sum == 20, "sum via const vget_components should be 20");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(manager_dense_vget_components_by_entity) {
    Manager<TestHealth> mgr;
    Entity ea{1};
    Entity eb{2};

    mgr.create(ComponentID{1}, ea, ComponentID{1}, 100);
    mgr.create(ComponentID{2}, ea, ComponentID{2}, 200);
    mgr.create(ComponentID{3}, eb, ComponentID{3}, 300);

    Manager<> &base = mgr;

    auto ea_comps = base.vget_components(ea);
    ADRIFT_SYNC_ASSERT(ea_comps.size() == 2, "vget_components(ea) should return 2 refs");
    for (auto &ref : ea_comps) {
        ADRIFT_SYNC_ASSERT(ref.get().owner_entity() == ea, "vget_components(ea) all should belong to ea");
    }

    auto eb_comps = base.vget_components(eb);
    ADRIFT_SYNC_ASSERT(eb_comps.size() == 1, "vget_components(eb) should return 1 ref");

    auto unknown = base.vget_components(Entity{99});
    ADRIFT_SYNC_ASSERT(unknown.empty(), "vget_components(unknown) should be empty");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(manager_dense_vget_components_by_entity_const) {
    Manager<TestHealth> mgr;
    Entity e1{10};
    Entity e2{20};

    mgr.create(ComponentID{1}, e1, ComponentID{1}, 1);
    mgr.create(ComponentID{2}, e2, ComponentID{2}, 2);

    const Manager<> &base = mgr;
    auto comps = base.vget_components(e1);
    ADRIFT_SYNC_ASSERT(comps.size() == 1, "const vget_components(e1) should return 1 ref");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(manager_sparse_vget_components_all) {
    Manager<TestMana> mgr;
    Entity e{1};

    mgr.create(ComponentID{1}, e, ComponentID{1}, 1.0f);
    mgr.create(ComponentID{2}, e, ComponentID{2}, 2.0f);

    Manager<> &base = mgr;
    auto comps = base.vget_components();
    ADRIFT_SYNC_ASSERT(comps.size() == 2, "sparse vget_components should return 2 refs");

    float sum = 0.0f;
    for (auto &ref : comps) {
        sum += ref.get().as<TestMana>().amount;
    }
    ADRIFT_SYNC_ASSERT(sum == 3.0f, "sum via sparse vget_components should be 3.0");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(manager_sparse_vget_components_by_entity) {
    Manager<TestMana> mgr;
    Entity e1{1};
    Entity e2{2};

    mgr.create(ComponentID{1}, e1, ComponentID{1}, 10.0f);
    mgr.create(ComponentID{2}, e2, ComponentID{2}, 20.0f);

    Manager<> &base = mgr;
    auto comps = base.vget_components(e1);
    ADRIFT_SYNC_ASSERT(comps.size() == 1, "sparse vget_components(e1) should return 1");
    ADRIFT_SYNC_ASSERT(comps[0].get().owner_entity() == e1, "should belong to e1");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(manager_type_mutate_is) {
    Manager<TestHealth> mgr;
    Manager<> &base = mgr;

    ADRIFT_SYNC_ASSERT(base.is<Manager<TestHealth>>(), "is<Manager<TestHealth>>() should return true");
    ADRIFT_SYNC_ASSERT(!base.is<Manager<TestMana>>(), "is<Manager<TestMana>>() should return false");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(manager_type_mutate_as) {
    Manager<TestHealth> mgr;
    Manager<> &base = mgr;

    auto &as_health = base.as<Manager<TestHealth>>();
    ADRIFT_SYNC_ASSERT(&as_health == &mgr, "as<Manager<TestHealth>>() should return same instance");

    // Verify we can use it
    Entity e{1};
    as_health.create(ComponentID{1}, e, ComponentID{1}, 42);
    ADRIFT_SYNC_ASSERT(as_health.get_component(ComponentID{1}).hp == 42, "as<>() returns functional manager");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(manager_type_mutate_try_as) {
    Manager<TestHealth> mgr;
    Manager<> &base = mgr;

    auto *ptr = base.try_as<Manager<TestHealth>>();
    ADRIFT_SYNC_ASSERT(ptr != nullptr, "try_as<Manager<TestHealth>>() should return non-null");

    auto *null_ptr = base.try_as<Manager<TestMana>>();
    ADRIFT_SYNC_ASSERT(null_ptr == nullptr, "try_as<wrong type>() should return nullptr");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(manager_type_mutate_const_try_as) {
    Manager<TestHealth> mgr;
    const Manager<> &base = mgr;

    const auto *ptr = base.try_as<Manager<TestHealth>>();
    ADRIFT_SYNC_ASSERT(ptr != nullptr, "const try_as<correct>() should return non-null");

    const auto *null_ptr = base.try_as<Manager<TestMana>>();
    ADRIFT_SYNC_ASSERT(null_ptr == nullptr, "const try_as<wrong>() should return nullptr");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(manager_type_mutate_is_by_type_id) {
    Manager<TestHealth> mgr;
    Manager<> &base = mgr;

    ADRIFT_SYNC_ASSERT(base.is(type_id::of<Manager<TestHealth>>()), "is(type_id) should return true");
    ADRIFT_SYNC_ASSERT(!base.is(type_id::of<Manager<TestMana>>()), "is(type_id) wrong type should be false");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(manager_type_mutate_type_accessor) {
    Manager<TestHealth> mgr;
    Manager<> &base = mgr;

    ADRIFT_SYNC_ASSERT(
        base.type() == type_id::of<Manager<TestHealth>>(), "type() should return correct type_id");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(manager_dense_empty_get_components) {
    Manager<TestHealth> mgr;
    ADRIFT_SYNC_ASSERT(
        std::ranges::empty(mgr.get_components()), "empty manager get_components should be empty");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(manager_dense_empty_vget_components) {
    Manager<TestHealth> mgr;
    Manager<> &base = mgr;
    ADRIFT_SYNC_ASSERT(base.vget_components().empty(), "empty manager vget_components should be empty");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(manager_dense_empty_get_components_by_entity) {
    Manager<TestHealth> mgr;
    ADRIFT_SYNC_ASSERT(
        std::ranges::empty(mgr.get_components(Entity{1})),
        "empty manager get_components(entity) should be empty");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(manager_dense_empty_vget_components_by_entity) {
    Manager<TestHealth> mgr;
    Manager<> &base = mgr;
    ADRIFT_SYNC_ASSERT(
        base.vget_components(Entity{1}).empty(), "empty manager vget_components(entity) should be empty");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(manager_sparse_empty_get_components) {
    Manager<TestMana> mgr;
    ADRIFT_SYNC_ASSERT(std::ranges::empty(mgr.get_components()), "empty sparse manager get_components empty");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(manager_sparse_empty_vget_components) {
    Manager<TestMana> mgr;
    Manager<> &base = mgr;
    ADRIFT_SYNC_ASSERT(base.vget_components().empty(), "empty sparse manager vget_components empty");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(manager_dense_many_components) {
    Manager<TestHealth> mgr;
    Entity e{1};
    constexpr int N = 100;

    for (int i = 1; i <= N; ++i) {
        mgr.create(ComponentID{static_cast<u64>(i)}, e, ComponentID{static_cast<u64>(i)}, i);
    }

    int count = 0;
    int sum = 0;
    for (auto &c : mgr.get_components()) {
        sum += c.hp;
        count++;
    }
    ADRIFT_SYNC_ASSERT(count == N, "should have {} components, got {}", N, count);
    ADRIFT_SYNC_ASSERT(sum == N * (N + 1) / 2, "sum should be 1+2+...+N");

    // Spot-check individual components
    ADRIFT_SYNC_ASSERT(mgr.get_component(ComponentID{1}).hp == 1, "component 1");
    ADRIFT_SYNC_ASSERT(mgr.get_component(ComponentID{static_cast<u64>(N)}).hp == N, "component N");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(manager_sparse_many_components) {
    Manager<TestMana> mgr;
    Entity e{1};
    constexpr int N = 50;

    for (int i = 1; i <= N; ++i) {
        mgr.create(
            ComponentID{static_cast<u64>(i)}, e, ComponentID{static_cast<u64>(i)}, static_cast<float>(i));
    }

    int count = 0;
    for (auto &c : mgr.get_components()) {
        (void)c;
        count++;
    }
    ADRIFT_SYNC_ASSERT(count == N, "sparse should have N components");

    ADRIFT_SYNC_ASSERT(mgr.get_component(ComponentID{1}).amount == 1.0f, "sparse component 1");
    ADRIFT_SYNC_ASSERT(
        mgr.get_component(ComponentID{static_cast<u64>(N)}).amount == static_cast<float>(N),
        "sparse component N");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(manager_dense_destroy_reflected_in_iteration) {
    Manager<TestHealth> mgr;
    Entity e{1};

    mgr.create(ComponentID{1}, e, ComponentID{1}, 10);
    mgr.create(ComponentID{2}, e, ComponentID{2}, 20);
    mgr.create(ComponentID{3}, e, ComponentID{3}, 30);

    mgr.destroy(ComponentID{2});

    int count = 0;
    int sum = 0;
    for (auto &c : mgr.get_components()) {
        sum += c.hp;
        count++;
    }
    ADRIFT_SYNC_ASSERT(count == 2, "after destroy, iteration should yield 2 components");
    ADRIFT_SYNC_ASSERT(sum == 40, "remaining sum should be 10 + 30 = 40");

    // vget_components should also reflect the change
    Manager<> &base = mgr;
    auto vcomps = base.vget_components();
    ADRIFT_SYNC_ASSERT(vcomps.size() == 2, "vget_components after destroy should yield 2");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(manager_const_get_component) {
    Manager<TestHealth> mgr;
    Entity e{1};
    ComponentID id{1};

    mgr.create(id, e, id, 77);

    const auto &cmgr = mgr;
    const auto &comp = cmgr.get_component(id);
    ADRIFT_SYNC_ASSERT(comp.hp == 77, "const get_component should return correct data");
    ADRIFT_SYNC_ASSERT(comp.owner_entity() == e, "const get_component should report correct owner");
    ADRIFT_SYNC_SUCCESS();
}
