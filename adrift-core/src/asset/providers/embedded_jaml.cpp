// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#include "adrift/core/asset/asset_provider.h"

#include <string_view>
#include <vector>

#include "adrift/assert.h"
#include "adrift/async/future.h"
#include "adrift/container/hash_map.h"
#include "adrift/core/asset/embedded_tree.h"
#include "adrift/sync/rwspinlock.h"
#include "adrift/util/murmur.h"

namespace adrift::core::asset::providers {

namespace embedded_jaml {

static const EmbeddedAssetNode *g_embedded_asset_tree{nullptr};
static sync::rwspinlock<hash_map<AssetID, std::span<const std::byte>>> g_embedded_asset_cache{};

};  // namespace embedded_jaml

bool EmbeddedJamlProvider::set_embedded_asset_tree(EmbeddedAssetNode *tree) {
    if (embedded_jaml::g_embedded_asset_tree != nullptr) {
        panic("资产树已被设置，不能重复设置");
    }

    embedded_jaml::g_embedded_asset_tree = tree;
    return true;
}

EmbeddedJamlProvider::EmbeddedJamlProvider() {
    // TODO: 日志 info[索引 EmbeddedJaml 资产数据]

    struct indexer {
        static async::future<> index(const EmbeddedAssetNode *list, u128 path_hash) {
            for (auto node = list; node; node = node->next) {
                ADRIFT_ASSERT(!node->children || node->data.empty());

                const u128 current_full_hash = path_hash ^ util::hash_str(node->name);

                if (node->children) {
                    co_await index(node->children, current_full_hash);
                    co_return;
                }

                embedded_jaml::g_embedded_asset_cache.write()->emplace(
                    AssetID{current_full_hash}, node->data);
            }
        }
    };

    indexer::index(embedded_jaml::g_embedded_asset_tree, 0).placement_execute();
}

async::future<std::expected<JamlSource, AssetLoadFailed>> EmbeddedJamlProvider::read_asset(AssetID id) {
    auto &jaml_source_bytes = *embedded_jaml::g_embedded_asset_cache.read()->get(id);
    auto jaml_source_str = ustr::format(
        "{}",
        std::string_view{reinterpret_cast<const char *>(jaml_source_bytes.data()), jaml_source_bytes.size()});
    JamlSource jaml_source{};
    jaml_source.provide_source(std::move(jaml_source_str));
    co_return std::move(jaml_source);
}

async::future<std::expected<void, AssetSaveFailed>>
EmbeddedJamlProvider::write_asset(AssetID, JamlTarget &&) {
    panic("EmbeddedJamlProvider 禁用资产写入");
}

};  // namespace adrift::core::asset::providers
