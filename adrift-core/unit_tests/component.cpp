// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#include "adrift/core/ecs/component.h"
#include "adrift/core/ecs/component_storage.h"
#include "adrift/core/ecs/entity.h"
#include "adrift/core/ecs/manager.h"
#include "adrift/test/test.h"

using namespace adrift;
using namespace adrift::core::ecs;

struct TestCompX : Component<TestCompX> {
    using Storage = DenseComponentStorage<TestCompX>;

    int value;

    TestCompX(Entity e, ComponentID id, int v)
            : Component<TestCompX>{e, id}
            , value{v} {}
};

struct TestCompY : Component<TestCompY> {
    using Storage = SparseComponentStorage<TestCompY>;

    float data;

    TestCompY(Entity e, ComponentID id, float d)
            : Component<TestCompY>{e, id}
            , data{d} {}
};

ADRIFT_SYNC_TEST(component_id_default_invalid) {
    ComponentID id;
    ADRIFT_SYNC_ASSERT(!id, "default constructed ComponentID should be invalid");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(component_id_construct_with_valid_u64) {
    ComponentID id{42};
    ADRIFT_SYNC_ASSERT(id, "ComponentID constructed with non-zero u64 should be valid");
    ADRIFT_SYNC_ASSERT(
        id.underlying() == 42, "underlying() should return the exact u64 used at construction");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(component_id_construct_with_zero_is_invalid) {
    ComponentID id{0};
    ADRIFT_SYNC_ASSERT(!id, "ComponentID{{0}} should be treated as invalid (INVALID sentinel)");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(component_id_equality) {
    ComponentID a{1};
    ComponentID b{1};
    ComponentID c{2};

    ADRIFT_SYNC_ASSERT(a == b, "ComponentIDs with same underlying value should be equal");
    ADRIFT_SYNC_ASSERT(!(a == c), "ComponentIDs with different underlying values should not be equal");
    ADRIFT_SYNC_ASSERT(
        !(a == ComponentID{}), "valid ComponentID should not equal default (invalid) ComponentID");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(component_id_copy_and_move) {
    ComponentID original{99};
    ComponentID copy{original};
    ADRIFT_SYNC_ASSERT(copy == original, "copied ComponentID should equal the original");
    ADRIFT_SYNC_ASSERT(copy.underlying() == 99, "copied ComponentID should preserve the underlying value");

    ComponentID moved{std::move(copy)};
    ADRIFT_SYNC_ASSERT(moved == original, "moved ComponentID should equal the original");
    ADRIFT_SYNC_ASSERT(moved.underlying() == 99, "moved ComponentID should preserve the underlying value");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(component_id_copy_assign) {
    ComponentID a{7};
    ComponentID b{3};
    b = a;
    ADRIFT_SYNC_ASSERT(b == a, "copy-assigned ComponentID should equal the source");
    ADRIFT_SYNC_ASSERT(
        b.underlying() == 7, "copy-assigned ComponentID should preserve the source underlying value");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(component_id_move_assign) {
    ComponentID a{55};
    ComponentID b{1};
    b = std::move(a);
    ADRIFT_SYNC_ASSERT(
        b.underlying() == 55, "move-assigned ComponentID should take the source underlying value");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(component_id_debug_output) {
    ComponentID id{1};
    auto s = id.debug();
    ADRIFT_SYNC_ASSERT(!s.is_empty(), "debug() should return a non-empty string for a valid id");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(component_id_debug_none) {
    ComponentID id;
    auto s = id.debug();
    ADRIFT_SYNC_ASSERT(!s.is_empty(), "debug() should return a non-empty string even for NONE id");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(entity_default_invalid) {
    Entity e;
    ADRIFT_SYNC_ASSERT(!e, "default constructed Entity should be invalid");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(entity_construct_with_valid_u64) {
    Entity e{100};
    ADRIFT_SYNC_ASSERT(e, "Entity constructed with non-zero u64 should be valid");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(entity_equality) {
    Entity a{10};
    Entity b{10};
    Entity c{20};

    ADRIFT_SYNC_ASSERT(a == b, "Entities with same underlying value should be equal");
    ADRIFT_SYNC_ASSERT(!(a == c), "Entities with different underlying values should not be equal");
    ADRIFT_SYNC_ASSERT(!(a == Entity{}), "valid Entity should not equal default (invalid) Entity");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(entity_copy_and_move) {
    Entity original{77};
    Entity copy{original};
    ADRIFT_SYNC_ASSERT(copy == original, "copied Entity should equal the original");

    Entity moved{std::move(copy)};
    ADRIFT_SYNC_ASSERT(moved == original, "moved Entity should equal the original");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(component_owner_entity) {
    Manager<TestCompX> mgr;
    Entity entity{42};
    ComponentID id{1};

    auto &comp = mgr.create(id, entity, id, 10);
    ADRIFT_SYNC_ASSERT(comp.owner_entity() == entity, "component should report correct owner entity");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(component_owner_entity_multiple) {
    Manager<TestCompX> mgr;
    Entity ea{1};
    Entity eb{2};

    mgr.create(ComponentID{1}, ea, ComponentID{1}, 10);
    mgr.create(ComponentID{2}, eb, ComponentID{2}, 20);

    ADRIFT_SYNC_ASSERT(mgr.get_component(ComponentID{1}).owner_entity() == ea, "component 1 belongs to ea");
    ADRIFT_SYNC_ASSERT(mgr.get_component(ComponentID{2}).owner_entity() == eb, "component 2 belongs to eb");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(component_type_mutate_is) {
    Manager<TestCompX> mgr;
    Entity e{1};
    ComponentID id{1};

    auto &comp = mgr.create(id, e, id, 42);
    Component<> &base = comp;

    ADRIFT_SYNC_ASSERT(base.is<TestCompX>(), "Component<>::is<TestCompX>() should return true");
    ADRIFT_SYNC_ASSERT(!base.is<TestCompY>(), "Component<>::is<TestCompY>() should return false");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(component_type_mutate_as) {
    Manager<TestCompX> mgr;
    Entity e{1};
    ComponentID id{1};

    auto &comp = mgr.create(id, e, id, 77);
    Component<> &base = comp;

    auto &as_x = base.as<TestCompX>();
    ADRIFT_SYNC_ASSERT(&as_x == &comp, "as<TestCompX>() should return the same instance");
    ADRIFT_SYNC_ASSERT(as_x.value == 77, "as<TestCompX>() should preserve data");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(component_type_mutate_try_as) {
    Manager<TestCompX> mgr;
    Entity e{1};
    ComponentID id{1};

    auto &comp = mgr.create(id, e, id, 33);
    Component<> &base = comp;

    auto *ptr = base.try_as<TestCompX>();
    ADRIFT_SYNC_ASSERT(ptr != nullptr, "try_as<TestCompX>() should return non-null pointer");
    ADRIFT_SYNC_ASSERT(ptr->value == 33, "try_as<TestCompX>() should return correct data");

    auto *null_ptr = base.try_as<TestCompY>();
    ADRIFT_SYNC_ASSERT(null_ptr == nullptr, "try_as<wrong type>() should return nullptr");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(component_type_mutate_const_try_as) {
    Manager<TestCompX> mgr;
    Entity e{1};
    ComponentID id{1};

    mgr.create(id, e, id, 55);
    const auto &comp = mgr.get_component(id);
    const Component<> &base = comp;

    const auto *ptr = base.try_as<TestCompX>();
    ADRIFT_SYNC_ASSERT(ptr != nullptr, "const try_as<correct type>() should return non-null pointer");
    ADRIFT_SYNC_ASSERT(ptr->value == 55, "const try_as should preserve data");

    const auto *null_ptr = base.try_as<TestCompY>();
    ADRIFT_SYNC_ASSERT(null_ptr == nullptr, "const try_as<wrong type>() should return nullptr");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(component_type_mutate_is_by_type_id) {
    Manager<TestCompX> mgr;
    Entity e{1};
    ComponentID id{1};

    auto &comp = mgr.create(id, e, id, 1);
    Component<> &base = comp;

    ADRIFT_SYNC_ASSERT(base.is(type_id::of<TestCompX>()), "is(type_id) should return true for correct type");
    ADRIFT_SYNC_ASSERT(!base.is(type_id::of<TestCompY>()), "is(type_id) should return false for wrong type");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(component_type_mutate_type_accessor) {
    Manager<TestCompX> mgr;
    Entity e{1};
    ComponentID id{1};

    auto &comp = mgr.create(id, e, id, 99);
    Component<> &base = comp;

    ADRIFT_SYNC_ASSERT(base.type() == type_id::of<TestCompX>(), "type() should return the correct type_id");
    ADRIFT_SYNC_SUCCESS();
}
