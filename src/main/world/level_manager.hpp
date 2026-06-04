#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "glm/glm/vec2.hpp"

#include "ecs/ecs.hpp"
#include "utils/arena.hpp"
#include "utils/random.hpp"
#include "systems/spawn/periodic_spawner_object.hpp"
#include "systems/scene/scene_system.hpp"
#include "systems/transformation/transform_object.hpp"
#include "systems/defer/deferred_system.hpp"
#include "utils/save_system.hpp"

#include "scoreboard.hpp"
#include "score_hud.hpp"
#include "panel_occlusion_fx.hpp"
#include "shrooms_assets.hpp"
#include "game_audio.hpp"
#include "shrooms_screen.hpp"
#include "shrooms_texture_sizing.hpp"
#include "engine/geometry_builder.h"
#include "engine/resource_ids.h"
#include "systems/render/sprite_system.hpp"
#include "vfx.hpp"
#include "camera_shake.hpp"
#include "round_transition.hpp"
#include "level_intro.hpp"
#include "leaderboard.hpp"
#include "daily_runtime.hpp"

namespace levels {

enum class ObjectiveRule {
  CollectOnly,
  SortOnly,
};

enum class GameMode {
  Collector,
  Recipe,
};

struct SpawnerPlan {
  std::string type;
  std::string template_name;
  float period = 0.0f;
  double density = 0.0;
  int total_to_spawn = 0;
};

struct LevelDefinition {
  std::string id;
  std::vector<std::pair<std::string, int>> recipe_order;
  std::unordered_map<std::string, int> recipe;
  std::vector<SpawnerPlan> spawners;
  ObjectiveRule objective_rule = ObjectiveRule::CollectOnly;
  std::string objective_hint;
};

struct LastResult {
  bool success = false;
  int collected = 0;
  int sorted = 0;
  int global_score = 0;
  size_t level_index = 0;
  std::string level_id;
  bool infinite_mode = false;
  bool tutorial_mode = false;
  GameMode game_mode = GameMode::Collector;
  int rounds_won = 0;
  int round_index = 0;
};

enum class LossReason {
  None,
  TooMany,
  NotEnough,
  WrongAction,
  Dropped,
};

struct LossInfo {
  LossReason reason = LossReason::None;
  std::string type;
};

struct InfiniteCollectorTicket {
  std::string type;
  uint32_t index = 0;
};

inline std::vector<LevelDefinition> parsed_levels{};
inline std::vector<LevelDefinition> base_levels{};
inline std::unordered_map<std::string, periodic_spawn::PeriodicSpawnerObject*> spawners_by_type{};
inline std::unordered_map<std::string, std::unordered_set<ecs::Entity*>> active_entities{};
inline std::unordered_map<std::string, int> collected_counts{};
inline std::unordered_map<std::string, int> sorted_counts{};
inline size_t current_level_index = 0;
inline size_t unlocked_level_count = 0;
inline size_t last_played_level_index = 0;
inline std::string last_game_status = "No games yet";
inline bool last_game_success = false;
inline bool level_failed = false;
inline bool level_finished = false;
inline LastResult last_result{};
inline bool last_result_valid = false;
inline bool game_over_pending = false;
inline LossInfo pending_loss{};
inline LossInfo last_loss{};
inline bool infinite_mode = false;
inline bool tutorial_mode = false;
inline int infinite_round_index = 0;
inline int infinite_rounds_won = 0;
inline constexpr int kCollectorLivesPerRun = 3;
inline int collector_lives_remaining = kCollectorLivesPerRun;
inline int current_run_score = 0;
inline std::unordered_set<std::string> milestone_bonus_awarded{};
inline bool infinite_preview_ready = false;
inline std::string infinite_preview_date{};
inline LevelDefinition infinite_level{};
inline LevelDefinition tutorial_level{};
inline bool progress_save_exists = false;
inline bool tutorial_completed = false;
inline std::unordered_map<std::string, SpawnerPlan> base_spawner_plans{};
inline std::vector<std::string> infinite_types{};
inline render_system::SpriteRenderable* background_sprite = nullptr;
inline transform::NoRotationTransform* background_transform = nullptr;
inline GameMode current_game_mode = GameMode::Collector;
inline std::string current_daily_date{};
inline uint32_t current_daily_seed = 0;
inline std::vector<InfiniteCollectorTicket> infinite_collector_queue{};
inline uint32_t infinite_collector_ticket_index = 0;
inline std::vector<std::string> infinite_collector_type_bag{};
inline std::vector<std::string> infinite_collector_previous_type_bag{};
inline size_t infinite_collector_type_bag_pos = 0;
inline uint32_t infinite_collector_type_bag_index = 0;
inline constexpr size_t kInfiniteCollectorMinQueue = 3;
inline constexpr size_t kInfiniteCollectorMaxQueue = 5;
inline constexpr int kInfiniteCollectorScorePerLevel = 500;
inline constexpr int kInfiniteCollectorReferenceScore = 500;
inline constexpr int kInfiniteCollectorReferenceScorePerLevel = 150;
inline constexpr int kInfiniteCollectorFirstDensityLevel =
    kInfiniteCollectorReferenceScore / kInfiniteCollectorReferenceScorePerLevel;
inline constexpr int kInfiniteRecipeScorePerLevel = 450;

using TutorialSpawnHook = std::function<void(const std::string&, ecs::Entity*)>;
using TutorialCatchHook =
    std::function<void(const std::string&, ecs::Entity*, bool from_familiar)>;
using TutorialMissHook = std::function<void(const std::string&, ecs::Entity*)>;
using TutorialSortHook = std::function<void(const std::string&, ecs::Entity*)>;
inline TutorialSpawnHook tutorial_spawn_hook{};
inline TutorialCatchHook tutorial_catch_hook{};
inline TutorialMissHook tutorial_miss_hook{};
inline TutorialSortHook tutorial_sort_hook{};

constexpr const char* kLegacyProgressKey = "shrooms_progress";
constexpr const char* kSelectedModeKey = "shrooms_selected_mode";
inline constexpr size_t kTutorialLevelIndexOffset = 1;

struct ProgressSaveData {
  size_t unlocked_level_count = 0;
  bool has_unlocked_level_count = false;
  bool tutorial_completed = false;
  bool has_tutorial_completed = false;
};

inline const char* progress_key_for_mode(GameMode mode) {
  switch (mode) {
    case GameMode::Collector:
      return "shrooms_progress_collector";
    case GameMode::Recipe:
    default:
      return "shrooms_progress_recipe";
  }
}

inline const char* legacy_progress_key_for_mode(GameMode mode) {
  switch (mode) {
    case GameMode::Collector:
      return "shrooms_progress_easy";
    case GameMode::Recipe:
    default:
      return "shrooms_progress_normal";
  }
}

inline leaderboard::Profile leaderboard_profile_for_mode(GameMode mode) {
  switch (mode) {
    case GameMode::Collector:
      return leaderboard::Profile::Collector;
    case GameMode::Recipe:
    default:
      return leaderboard::Profile::Recipe;
  }
}

inline void set_tutorial_hooks(TutorialSpawnHook spawn_hook, TutorialCatchHook catch_hook,
                               TutorialMissHook miss_hook, TutorialSortHook sort_hook) {
  tutorial_spawn_hook = std::move(spawn_hook);
  tutorial_catch_hook = std::move(catch_hook);
  tutorial_miss_hook = std::move(miss_hook);
  tutorial_sort_hook = std::move(sort_hook);
}

inline GameMode game_mode() { return current_game_mode; }

inline std::string mode_label() {
  switch (current_game_mode) {
    case GameMode::Collector:
      return "Collector";
    case GameMode::Recipe:
    default:
      return "Recipe";
  }
}

inline bool is_tutorial_mode() { return tutorial_mode; }

inline bool is_collector_mode() { return current_game_mode == GameMode::Collector; }

inline bool is_recipe_mode() { return current_game_mode == GameMode::Recipe; }

inline bool shooting_enabled() { return tutorial_mode || is_recipe_mode(); }

inline bool is_infinite_collector_run() {
  return infinite_mode && current_game_mode == GameMode::Collector && !tutorial_mode;
}

inline int collector_lives() { return collector_lives_remaining; }

inline int score() { return current_run_score; }

inline int infinite_collector_level_for_score(int score_value) {
  return std::max(0, score_value) / kInfiniteCollectorScorePerLevel;
}

inline int infinite_collector_level_index() {
  return infinite_collector_level_for_score(current_run_score);
}

inline int infinite_collector_density_level_for_score(int score_value) {
  const int score_level = infinite_collector_level_for_score(score_value);
  if (score_level <= 0) return 0;
  return kInfiniteCollectorFirstDensityLevel + score_level - 1;
}

inline int infinite_collector_density_level_index() {
  return infinite_collector_density_level_for_score(current_run_score);
}

inline size_t tutorial_menu_index() {
  return parsed_levels.size() + kTutorialLevelIndexOffset;
}

inline const LevelDefinition* current_level() {
  if (infinite_mode) {
    return &infinite_level;
  }
  if (tutorial_mode) {
    return &tutorial_level;
  }
  if (parsed_levels.empty() || current_level_index >= parsed_levels.size()) {
    return nullptr;
  }
  return &parsed_levels[current_level_index];
}

inline size_t infinite_menu_index() {
  return parsed_levels.size();
}

inline bool infinite_available() {
  return !parsed_levels.empty();
}

inline size_t clamp_unlocked(size_t count) {
  if (parsed_levels.empty()) {
    return 0;
  }
  if (count < 1) {
    count = 1;
  }
  if (count > parsed_levels.size()) {
    count = parsed_levels.size();
  }
  return count;
}

inline bool is_unlocked(size_t index) {
  if (parsed_levels.empty()) return false;
  return index < unlocked_level_count;
}

inline size_t unlocked_levels() { return unlocked_level_count; }

inline bool has_progress_save() { return progress_save_exists; }

inline bool has_completed_tutorial() { return tutorial_completed; }

inline int progress_for_type(const LevelDefinition& level, const std::string& type) {
  switch (level.objective_rule) {
    case ObjectiveRule::SortOnly:
      return sorted_counts[type];
    case ObjectiveRule::CollectOnly:
    default:
      return collected_counts[type];
  }
}

inline std::string objective_line_text(const LevelDefinition& level, const std::string& type,
                                       int target) {
  if (!level.objective_hint.empty()) {
    return level.objective_hint + " " + std::to_string(target) + " " + type;
  }
  switch (level.objective_rule) {
    case ObjectiveRule::SortOnly:
      return "Shoot " + std::to_string(target) + " " + type;
    case ObjectiveRule::CollectOnly:
    default:
      return "Collect " + std::to_string(target) + " " + type;
  }
}

inline std::string mushroom_icon_texture(const std::string& type) {
  if (type == "mukhomor" || type == "lisi4ka" || type == "borovik") {
    return type + "_small";
  }
  return type;
}

inline void layout_background_sprite(const std::string& texture_name) {
  if (!background_sprite || !background_transform) return;
  if (texture_name.empty()) return;

  const float view_width = static_cast<float>(shrooms::screen::view_width);
  const float view_height = static_cast<float>(shrooms::screen::view_height);
  const glm::vec2 size = shrooms::texture_sizing::from_width_px(texture_name, view_width);
  // SpriteRenderable keeps its own quad geometry; resizing requires rebuilding it.
  background_sprite->geometry = engine::geometry::make_quad(size.x, size.y);
  background_sprite->uploaded = false;
  background_sprite->size = size;
  background_transform->pos = glm::vec2{0.0f, view_height - size.y};
}

inline void register_background_sprite(render_system::SpriteRenderable* sprite,
                                       transform::NoRotationTransform* transform,
                                       const std::string& texture_name) {
  background_sprite = sprite;
  background_transform = transform;
  layout_background_sprite(texture_name);
}

inline void set_background_texture(const std::string& texture_name) {
  if (!background_sprite) return;
  if (texture_name.empty()) return;
  const engine::TextureId tex_id = engine::resources::register_texture(texture_name);
  if (tex_id != engine::kInvalidTextureId) {
    background_sprite->texture_id = tex_id;
    layout_background_sprite(texture_name);
  }
}

inline std::string infinite_background_texture_for_round(int round_index) {
  if (parsed_levels.empty()) return "";
  const size_t index =
      static_cast<size_t>(std::max(0, round_index)) % parsed_levels.size();
  return parsed_levels[index].id;
}

inline void apply_infinite_background_for_round(int round_index) {
  set_background_texture(infinite_background_texture_for_round(round_index));
}

inline void apply_level_background(const LevelDefinition& level) {
  if (infinite_mode) {
    const int background_index =
        is_infinite_collector_run() ? infinite_collector_level_index() : infinite_round_index;
    apply_infinite_background_for_round(background_index);
    return;
  }
  if (tutorial_mode) {
    if (!parsed_levels.empty()) {
      set_background_texture(parsed_levels.front().id);
    } else {
      set_background_texture("level_1_eli");
    }
    return;
  }
  set_background_texture(level.id);
}

inline bool has_pending_failure() { return game_over_pending; }

inline const LossInfo& pending_loss_info() { return pending_loss; }

inline const LossInfo& last_loss_info() { return last_loss; }

inline std::string loss_reason_label(const LossInfo& info) {
  switch (info.reason) {
    case LossReason::TooMany: {
      if (!info.type.empty()) {
        return "Too many " + info.type;
      }
      return "Too many mushrooms";
    }
    case LossReason::NotEnough: {
      if (!info.type.empty()) {
        return "Not enough " + info.type;
      }
      return "Not enough mushrooms";
    }
    case LossReason::WrongAction: {
      if (!info.type.empty()) {
        return "Wrong action for " + info.type;
      }
      return "Wrong action";
    }
    case LossReason::Dropped: {
      if (current_game_mode == GameMode::Collector) {
        return "No lives left";
      }
      if (!info.type.empty()) {
        return info.type + " hit the floor";
      }
      return "Mushroom hit the floor";
    }
    case LossReason::None:
    default:
      return "Game over";
  }
}

inline bool parse_bool_value(const std::string& value, bool& out) {
  if (value == "1" || value == "true" || value == "True") {
    out = true;
    return true;
  }
  if (value == "0" || value == "false" || value == "False") {
    out = false;
    return true;
  }
  return false;
}

inline void parse_progress_save_line(std::string line, ProgressSaveData& data) {
  if (!line.empty() && line.back() == '\r') {
    line.pop_back();
  }
  if (line.empty()) return;

  const size_t delimiter = line.find('=');
  if (delimiter == std::string::npos) {
    if (data.has_unlocked_level_count) return;
    std::istringstream in(line);
    size_t count = 0;
    if (in >> count) {
      data.unlocked_level_count = count;
      data.has_unlocked_level_count = true;
    }
    return;
  }

  const std::string key = line.substr(0, delimiter);
  const std::string value = line.substr(delimiter + 1);
  if (key == "unlocked") {
    std::istringstream in(value);
    size_t count = 0;
    if (in >> count) {
      data.unlocked_level_count = count;
      data.has_unlocked_level_count = true;
    }
    return;
  }
  if (key == "tutorial_completed") {
    bool completed = false;
    if (parse_bool_value(value, completed)) {
      data.tutorial_completed = completed;
      data.has_tutorial_completed = true;
    }
  }
}

inline ProgressSaveData parse_progress_save(const std::string& text) {
  ProgressSaveData data{};
  std::istringstream lines(text);
  std::string line;
  while (std::getline(lines, line)) {
    parse_progress_save_line(line, data);
  }
  return data;
}

inline std::string serialize_progress_save() {
  std::ostringstream out;
  out << unlocked_level_count << '\n';
  out << "version=2\n";
  out << "unlocked=" << unlocked_level_count << '\n';
  out << "tutorial_completed=" << (tutorial_completed ? 1 : 0) << '\n';
  return out.str();
}

inline std::optional<std::string> read_progress_text_for_mode(GameMode mode,
                                                              bool include_shared_legacy) {
  auto saved = save::read_text(progress_key_for_mode(mode));
  if (!saved) {
    saved = save::read_text(legacy_progress_key_for_mode(mode));
  }
  if (!saved && include_shared_legacy) {
    saved = save::read_text(kLegacyProgressKey);
  }
  return saved;
}

inline bool progress_text_marks_tutorial_completed(const std::string& text) {
  const ProgressSaveData data = parse_progress_save(text);
  if (data.has_tutorial_completed) {
    return data.tutorial_completed;
  }
  return data.has_unlocked_level_count;
}

inline bool read_any_tutorial_completed_save() {
  if (auto saved = read_progress_text_for_mode(GameMode::Collector, false)) {
    if (progress_text_marks_tutorial_completed(*saved)) return true;
  }
  if (auto saved = read_progress_text_for_mode(GameMode::Recipe, false)) {
    if (progress_text_marks_tutorial_completed(*saved)) return true;
  }
  if (auto saved = save::read_text(kLegacyProgressKey)) {
    if (progress_text_marks_tutorial_completed(*saved)) return true;
  }
  return false;
}

inline void save_progress() {
  if (parsed_levels.empty()) return;
  save::write_text(progress_key_for_mode(current_game_mode), serialize_progress_save());
  progress_save_exists = true;
}

inline void load_progress() {
  unlocked_level_count = parsed_levels.empty() ? 0 : 1;
  auto saved = read_progress_text_for_mode(current_game_mode, true);
  progress_save_exists = saved.has_value();
  tutorial_completed = read_any_tutorial_completed_save();
  if (!saved) return;
  const ProgressSaveData data = parse_progress_save(*saved);
  if (!data.has_unlocked_level_count) {
    return;
  }
  unlocked_level_count = clamp_unlocked(data.unlocked_level_count);
  if (data.has_tutorial_completed) {
    tutorial_completed = tutorial_completed || data.tutorial_completed;
  } else {
    tutorial_completed = true;
  }
}

inline void save_selected_mode() {
  save::write_text(kSelectedModeKey,
                   current_game_mode == GameMode::Collector ? "collector" : "recipe");
}

inline GameMode load_selected_mode() {
  auto saved = save::read_text(kSelectedModeKey);
  if (!saved) return GameMode::Collector;
  if (*saved == "recipe" || *saved == "Recipe") {
    return GameMode::Recipe;
  }
  return GameMode::Collector;
}

inline void unlock_next_level(size_t level_index) {
  if (parsed_levels.empty()) return;
  const size_t target = level_index + 2;
  const size_t clamped = clamp_unlocked(target);
  if (clamped > unlocked_level_count) {
    unlocked_level_count = clamped;
    save_progress();
  }
}

inline void mark_tutorial_completed() {
  if (tutorial_completed && progress_save_exists) return;
  tutorial_completed = true;
  save_progress();
}

inline void reset_active_entities() {
  for (auto& [type, entities] : active_entities) {
    for (auto* entity : entities) {
      if (entity) {
        entity->mark_deleted();
      }
    }
  }
  active_entities.clear();
}

inline uint32_t fnv1a_append(uint32_t hash, const std::string& text) {
  for (unsigned char c : text) {
    hash ^= static_cast<uint32_t>(c);
    hash *= 16777619u;
  }
  return hash;
}

inline std::string mode_seed_tag() {
  switch (current_game_mode) {
    case GameMode::Collector:
      return "collector";
    case GameMode::Recipe:
    default:
      return "recipe";
  }
}

inline void refresh_daily_seed_if_needed() {
  const std::string today = daily_runtime::local_calendar_date().iso_yyyy_mm_dd();
  const std::string salt = std::string("shrooms_daily_infinite_v2_") + mode_seed_tag();
  const uint32_t seed = daily_runtime::day_hash(salt, today);
  if (today == current_daily_date && seed == current_daily_seed) {
    return;
  }
  current_daily_date = today;
  current_daily_seed = seed;
}

inline uint32_t hash_daily_round(uint32_t stream, int round_index) {
  refresh_daily_seed_if_needed();
  uint32_t hash = current_daily_seed;
  hash ^= stream * 0x9e3779b9u;
  hash ^= static_cast<uint32_t>(round_index + 1) * 0x85ebca6bu;
  hash *= 16777619u;
  return hash;
}

inline uint32_t hash_daily_ticket(uint32_t stream, uint32_t ticket_index) {
  refresh_daily_seed_if_needed();
  uint32_t hash = current_daily_seed;
  hash ^= stream * 0x9e3779b9u;
  hash ^= (ticket_index + 1u) * 0x85ebca6bu;
  hash *= 16777619u;
  return hash;
}

inline uint32_t mix_daily_hash(uint32_t hash) {
  hash ^= hash >> 16;
  hash *= 0x7feb352du;
  hash ^= hash >> 15;
  hash *= 0x846ca68bu;
  hash ^= hash >> 16;
  return hash;
}

inline uint32_t seed_for_level(const LevelDefinition& level, size_t seed_index,
                               const std::string& spawner_type) {
  uint32_t hash = 2166136261u;
  hash = fnv1a_append(hash, level.id);
  hash = fnv1a_append(hash, spawner_type);
  if (level.id == "Infinite") {
    refresh_daily_seed_if_needed();
    hash = fnv1a_append(hash, current_daily_date);
    hash ^= current_daily_seed;
  }
  hash ^= static_cast<uint32_t>(seed_index + 1) * 0x9e3779b9u;
  return hash;
}

inline void seed_spawners_for_level(const LevelDefinition& level, size_t seed_index) {
  if (is_infinite_collector_run()) {
    return;
  }
  for (const auto& plan : level.spawners) {
    auto it = spawners_by_type.find(plan.type);
    if (it == spawners_by_type.end()) continue;
    auto* spawner = it->second;
    if (!spawner) continue;
    const uint32_t seed = seed_for_level(level, seed_index, plan.type);
    spawner->reseed(seed);
  }
}

inline void build_infinite_spawner_cache() {
  base_spawner_plans.clear();
  infinite_types.clear();
  for (const auto& level : parsed_levels) {
    for (const auto& plan : level.spawners) {
      if (base_spawner_plans.find(plan.type) != base_spawner_plans.end()) continue;
      base_spawner_plans[plan.type] = plan;
      infinite_types.push_back(plan.type);
    }
  }
}

inline void reset_level_defaults(LevelDefinition& level) {
  level.objective_rule = ObjectiveRule::CollectOnly;
  level.objective_hint.clear();
}

inline void apply_collector_profile(LevelDefinition& level) {
  level.recipe.clear();
  level.recipe_order.clear();
  level.objective_rule = ObjectiveRule::CollectOnly;
  level.objective_hint = "Catch every mushroom";
}

inline void apply_mode_to_levels() {
  parsed_levels = base_levels;
  for (auto& level : parsed_levels) {
    reset_level_defaults(level);
  }
  if (current_game_mode == GameMode::Collector) {
    for (auto& level : parsed_levels) {
      apply_collector_profile(level);
    }
  }
  unlocked_level_count = clamp_unlocked(unlocked_level_count);
  build_infinite_spawner_cache();
}

inline void set_game_mode(GameMode new_mode) {
  if (current_game_mode == new_mode) return;
  current_game_mode = new_mode;
  save_selected_mode();
  current_daily_date.clear();
  current_daily_seed = 0;
  infinite_preview_ready = false;
  apply_mode_to_levels();
  load_progress();
  leaderboard::set_profile(leaderboard_profile_for_mode(current_game_mode));
  refresh_daily_seed_if_needed();
}

inline void cycle_game_mode() {
  set_game_mode(current_game_mode == GameMode::Collector ? GameMode::Recipe
                                                         : GameMode::Collector);
}

inline int infinite_recipe_score_level() {
  return std::max(0, current_run_score) / kInfiniteRecipeScorePerLevel;
}

inline int infinite_target_for_round_type(int round_index, const std::string& type) {
  const int score_boost = infinite_recipe_score_level();
  const int base = 2 + score_boost;
  uint32_t hash = hash_daily_round(0x41a13u, round_index);
  hash = fnv1a_append(hash, type);
  const int jitter = static_cast<int>(hash % 5u) - 1;
  return std::max(1, base + jitter);
}

inline SpawnerPlan scale_infinite_recipe_spawner(SpawnerPlan plan, int target) {
  const int score_level = infinite_recipe_score_level();
  const double density_multiplier = 1.0 + static_cast<double>(score_level) * 0.1;
  const float period_divisor = 1.0f + static_cast<float>(score_level) * 0.04f;
  plan.density = std::max(0.1, plan.density * density_multiplier);
  plan.period = std::max(0.7f, plan.period / period_divisor);
  const int spare = std::max(2 + score_level / 2, target / 2);
  plan.total_to_spawn = std::max(1, target + spare);
  return plan;
}

inline void build_infinite_level(int round_index) {
  refresh_daily_seed_if_needed();
  infinite_level = LevelDefinition{};
  infinite_level.id = "Infinite";
  infinite_level.recipe.clear();
  infinite_level.recipe_order.clear();
  infinite_level.spawners.clear();

  if (infinite_types.empty()) {
    build_infinite_spawner_cache();
  }
  if (infinite_types.empty()) {
    return;
  }

  for (const auto& type : infinite_types) {
    const int target = infinite_target_for_round_type(round_index, type);
    if (current_game_mode == GameMode::Recipe) {
      infinite_level.recipe[type] = target;
      infinite_level.recipe_order.emplace_back(type, target);
    }

    SpawnerPlan plan{};
    plan.type = type;
    auto base_it = base_spawner_plans.find(type);
    if (base_it != base_spawner_plans.end()) {
      plan.template_name = base_it->second.template_name;
      plan.period = base_it->second.period;
      plan.density = base_it->second.density;
    } else {
      plan.template_name = type + "_spawned";
      plan.period = 2.0f;
      plan.density = 0.7;
    }
    if (current_game_mode == GameMode::Collector) {
      plan.total_to_spawn = -1;
    } else {
      plan = scale_infinite_recipe_spawner(plan, target);
    }
    infinite_level.spawners.push_back(plan);
  }
  infinite_level.objective_rule = ObjectiveRule::CollectOnly;
  infinite_level.objective_hint =
      current_game_mode == GameMode::Collector ? "Catch mushrooms until your lives run out" : "";
}

inline void refill_infinite_collector_type_bag(const std::string& previous_type) {
  if (infinite_types.empty()) {
    build_infinite_spawner_cache();
  }
  infinite_collector_type_bag = infinite_types;
  infinite_collector_type_bag_pos = 0;

  if (infinite_collector_type_bag.size() <= 1) {
    infinite_collector_previous_type_bag = infinite_collector_type_bag;
    return;
  }

  uint32_t hash = hash_daily_ticket(0x7235u, infinite_collector_type_bag_index++);
  for (size_t i = infinite_collector_type_bag.size(); i > 1; --i) {
    hash = mix_daily_hash(hash + static_cast<uint32_t>(i));
    const size_t swap_index = static_cast<size_t>(hash % static_cast<uint32_t>(i));
    std::swap(infinite_collector_type_bag[i - 1], infinite_collector_type_bag[swap_index]);
  }

  if (infinite_collector_type_bag == infinite_collector_previous_type_bag) {
    const size_t rotate_by =
        1 + static_cast<size_t>(mix_daily_hash(hash) %
                                static_cast<uint32_t>(infinite_collector_type_bag.size() - 1));
    std::rotate(infinite_collector_type_bag.begin(),
                infinite_collector_type_bag.begin() + static_cast<std::ptrdiff_t>(rotate_by),
                infinite_collector_type_bag.end());
  }

  if (!previous_type.empty() && infinite_collector_type_bag.front() == previous_type) {
    const size_t swap_index =
        1 + static_cast<size_t>(mix_daily_hash(hash ^ 0xa511e9b3u) %
                                static_cast<uint32_t>(infinite_collector_type_bag.size() - 1));
    std::swap(infinite_collector_type_bag.front(), infinite_collector_type_bag[swap_index]);
  }

  infinite_collector_previous_type_bag = infinite_collector_type_bag;
}

inline std::string next_infinite_collector_type(const std::string& previous_type) {
  if (infinite_types.empty()) {
    build_infinite_spawner_cache();
  }
  if (infinite_types.empty()) {
    return "";
  }
  if (infinite_collector_type_bag_pos >= infinite_collector_type_bag.size()) {
    refill_infinite_collector_type_bag(previous_type);
  }
  if (infinite_collector_type_bag_pos >= infinite_collector_type_bag.size()) {
    return "";
  }
  return infinite_collector_type_bag[infinite_collector_type_bag_pos++];
}

inline uint32_t infinite_collector_seed_for_ticket(const InfiniteCollectorTicket& ticket) {
  uint32_t hash = hash_daily_ticket(0x9f41u, ticket.index);
  hash = fnv1a_append(hash, ticket.type);
  return hash;
}

inline void refill_infinite_collector_queue() {
  if (infinite_types.empty()) {
    build_infinite_spawner_cache();
  }
  while (infinite_collector_queue.size() < kInfiniteCollectorMaxQueue &&
         !infinite_types.empty()) {
    const uint32_t ticket_index = infinite_collector_ticket_index++;
    const std::string previous_type =
        infinite_collector_queue.empty() ? "" : infinite_collector_queue.back().type;
    const std::string type = next_infinite_collector_type(previous_type);
    if (type.empty()) break;
    infinite_collector_queue.push_back(InfiniteCollectorTicket{type, ticket_index});
  }
}

inline void reset_infinite_collector_queue() {
  infinite_collector_queue.clear();
  infinite_collector_ticket_index = 0;
  infinite_collector_type_bag.clear();
  infinite_collector_previous_type_bag.clear();
  infinite_collector_type_bag_pos = 0;
  infinite_collector_type_bag_index = 0;
  refill_infinite_collector_queue();
}

inline SpawnerPlan infinite_collector_plan_for_type(const std::string& type) {
  auto it = base_spawner_plans.find(type);
  if (it != base_spawner_plans.end()) {
    return it->second;
  }
  SpawnerPlan plan{};
  plan.type = type;
  plan.template_name = type + "_spawned";
  plan.period = 2.0f;
  plan.density = 0.7;
  plan.total_to_spawn = 1;
  return plan;
}

inline SpawnerPlan scale_infinite_collector_spawner(SpawnerPlan plan) {
  const int collector_level = infinite_collector_density_level_index();
  const int wave_level = infinite_collector_level_index();
  const double density_multiplier = 1.0 + static_cast<double>(collector_level) * 0.25;
  plan.density = std::max(0.1, plan.density * density_multiplier);
  plan.total_to_spawn = std::max(1, 1 + wave_level);
  return plan;
}

inline void disable_all_spawners() {
  for (auto& [_, spawner] : spawners_by_type) {
    if (spawner) {
      spawner->enabled = false;
    }
  }
}

inline void activate_infinite_collector_front_spawner() {
  disable_all_spawners();
  refill_infinite_collector_queue();

  while (!infinite_collector_queue.empty()) {
    const auto& ticket = infinite_collector_queue.front();
    auto spawner_it = spawners_by_type.find(ticket.type);
    if (spawner_it == spawners_by_type.end() || !spawner_it->second) {
      infinite_collector_queue.erase(infinite_collector_queue.begin());
      refill_infinite_collector_queue();
      continue;
    }

    const SpawnerPlan plan =
        scale_infinite_collector_spawner(infinite_collector_plan_for_type(ticket.type));
    auto* spawner = spawner_it->second;
    spawner->configure(plan.period, plan.density, plan.total_to_spawn);
    spawner->reseed(infinite_collector_seed_for_ticket(ticket));
    spawner->enabled = true;
    return;
  }
}

inline void advance_infinite_collector_ticket_if_depleted(const std::string& type) {
  if (!is_infinite_collector_run()) return;
  if (infinite_collector_queue.empty() || infinite_collector_queue.front().type != type) {
    return;
  }
  auto spawner_it = spawners_by_type.find(type);
  if (spawner_it == spawners_by_type.end() || !spawner_it->second ||
      !spawner_it->second->is_depleted()) {
    return;
  }

  infinite_collector_queue.erase(infinite_collector_queue.begin());
  refill_infinite_collector_queue();
  if (infinite_collector_queue.size() < kInfiniteCollectorMinQueue) {
    refill_infinite_collector_queue();
  }
  activate_infinite_collector_front_spawner();
}

inline void on_infinite_collector_ticket_spawned(const std::string& type) {
  if (!is_infinite_collector_run()) return;
  deferred::fire_deferred(
      [type]() { advance_infinite_collector_ticket_if_depleted(type); }, 0);
}

inline void refresh_infinite_collector_progression_for_score(int previous_score,
                                                             int next_score) {
  if (!is_infinite_collector_run()) return;
  const int previous_level = infinite_collector_level_for_score(previous_score);
  const int next_level = infinite_collector_level_for_score(next_score);
  if (previous_level == next_level) return;
  apply_infinite_background_for_round(next_level);
  activate_infinite_collector_front_spawner();
}

inline void prepare_infinite_preview() {
  refresh_daily_seed_if_needed();
  infinite_round_index = 0;
  infinite_rounds_won = 0;
  build_infinite_level(infinite_round_index);
  infinite_preview_date = current_daily_date;
  infinite_preview_ready = true;
}

inline void parse_levels(const std::string& filename) {
  parsed_levels.clear();
  base_levels.clear();
  std::ifstream in(filename);
  if (!in.is_open()) {
    std::cerr << "Failed to open level config: " << filename << std::endl;
    return;
  }

  LevelDefinition current{};
  std::string token;
  while (in >> token) {
    if (token == "level") {
      if (!current.id.empty()) {
        parsed_levels.push_back(current);
      }
      current = LevelDefinition{};
      in >> current.id;
    } else if (token == "recipe") {
      std::string type;
      int count = 0;
      in >> type >> count;
      current.recipe[type] = count;
      current.recipe_order.emplace_back(type, count);
    } else if (token == "spawn") {
      SpawnerPlan plan{};
      in >> plan.type >> plan.template_name >> plan.period >> plan.density >> plan.total_to_spawn;
      current.spawners.push_back(plan);
    } else if (token == "level_end") {
      if (!current.id.empty()) {
        parsed_levels.push_back(current);
      }
      current = LevelDefinition{};
    }
  }
  if (!current.id.empty()) {
    parsed_levels.push_back(current);
  }
  base_levels = parsed_levels;
  apply_mode_to_levels();
}

inline void register_spawner(periodic_spawn::PeriodicSpawnerObject* spawner) {
  if (!spawner) return;
  if (spawner->spawn_type.empty()) {
    std::cerr << "Spawner registered without type, ignoring" << std::endl;
    return;
  }
  spawners_by_type[spawner->spawn_type] = spawner;
}

inline void update_scoreboard_for(const std::string& type) {
  auto* level = current_level();
  if (!level) return;
  auto target_it = level->recipe.find(type);
  if (target_it == level->recipe.end()) return;
  scoreboard::update_score(type, progress_for_type(*level, type), target_it->second);
}

inline constexpr int kScoreCatch = 10;
inline constexpr int kScoreSort = 7;
inline constexpr int kScoreMiss = -200;
inline constexpr int kScoreMilestone = 100;
inline constexpr int kScoreRoundAdvance = 100;

inline glm::vec2 score_anchor_for_entity(ecs::Entity* entity) {
  if (entity) {
    const glm::vec2 size = vfx::entity_size(entity);
    if (size.x > 0.0f && size.y > 0.0f) {
      return vfx::entity_center(entity, size);
    }
  }
  return shrooms::screen::norm_to_pixels(glm::vec2{0.0f, 0.0f});
}

inline int apply_score_delta(int delta, const glm::vec2& anchor, bool show_popup = true) {
  const int previous = current_run_score;
  current_run_score = std::max(0, current_run_score + delta);
  const int applied = current_run_score - previous;
  scoreboard::set_score(current_run_score);
  refresh_infinite_collector_progression_for_score(previous, current_run_score);
  if (show_popup && applied != 0) {
    vfx::spawn_score_delta(anchor, applied);
  }
  return applied;
}

inline void maybe_award_recipe_milestone(const std::string& type, int progress, int target,
                                         const glm::vec2& anchor, bool allow_score_effects) {
  if (!allow_score_effects) return;
  if (progress != target) return;
  if (!milestone_bonus_awarded.insert(type).second) return;
  apply_score_delta(kScoreMilestone, anchor, true);
}

inline void check_completion();
inline void trigger_failure(LossReason reason, const std::string& type = "");
inline void finalize_level(bool success);

inline bool uses_recipe_targets() {
  return current_game_mode == GameMode::Recipe && !tutorial_mode;
}

inline void reset_collector_lives_if_needed() {
  if (current_game_mode != GameMode::Collector || tutorial_mode) {
    collector_lives_remaining = 0;
    scoreboard::set_hearts_visible(false);
    return;
  }
  if (!infinite_mode || infinite_round_index == 0) {
    collector_lives_remaining = kCollectorLivesPerRun;
  }
  scoreboard::set_hearts_visible(true);
  scoreboard::set_hearts(collector_lives_remaining);
}

inline void decrement_collector_life() {
  if (current_game_mode != GameMode::Collector || tutorial_mode) return;
  collector_lives_remaining = std::max(0, collector_lives_remaining - 1);
  scoreboard::set_hearts(collector_lives_remaining);
}

inline void finalize_success_after_transition() {
  level_failed = false;
  level_finished = true;
  finalize_level(true);
}

inline void start_level_completed_transition() {
  if (level_finished || game_over_pending) return;
  if (round_transition::is_active()) return;
  round_transition::start_level_completed([]() { finalize_success_after_transition(); });
}

inline ecs::Entity* spawn_mushroom_now(const std::string& type, const glm::vec2& center_px) {
  auto it = spawners_by_type.find(type);
  if (it == spawners_by_type.end() || !it->second) return nullptr;
  auto* spawner = it->second;
  return spawner->rule.spawn(center_px);
}

inline std::string default_spawn_type() {
  for (const auto& level : parsed_levels) {
    if (!level.spawners.empty()) {
      return level.spawners.front().type;
    }
  }
  if (!spawners_by_type.empty()) {
    return spawners_by_type.begin()->first;
  }
  return "mukhomor";
}

inline void on_mushroom_spawned(const std::string& type, ecs::Entity* entity) {
  panel_occlusion_fx::attach_mushroom_mask(entity);
  if (tutorial_spawn_hook) {
    tutorial_spawn_hook(type, entity);
  }
  if (!current_level()) return;
  active_entities[type].insert(entity);
  on_infinite_collector_ticket_spawned(type);
}

inline void on_mushroom_caught(
    const std::string& type, ecs::Entity* entity,
    glm::vec2 player_center = glm::vec2{std::numeric_limits<float>::quiet_NaN(),
                                        std::numeric_limits<float>::quiet_NaN()},
    bool from_familiar = false,
    const transform::NoRotationTransform* catch_x_anchor = nullptr) {
  if (!entity || entity->is_pending_deletion()) return;
  if (vfx::is_mushroom_vfx_locked(entity)) return;
  if (tutorial_catch_hook) {
    tutorial_catch_hook(type, entity, from_familiar);
  }
  shrooms::audio::play_mushroom_catch();
  auto* level = current_level();
  vfx::spawn_catch_effect(entity, player_center, catch_x_anchor);
  if (!level) return;
  const glm::vec2 score_anchor = score_anchor_for_entity(entity);
  const bool score_enabled = !tutorial_mode;

  auto active_it = active_entities.find(type);
  if (active_it != active_entities.end()) {
    active_it->second.erase(entity);
  }
  camera_shake::add_trauma(0.12f);

  auto recipe_it = level->recipe.find(type);
  if (recipe_it == level->recipe.end()) {
    if (current_game_mode == GameMode::Collector && !tutorial_mode) {
      if (score_enabled) {
        apply_score_delta(kScoreCatch, score_anchor, true);
      }
      collected_counts[type] += 1;
    }
    check_completion();
    return;
  }
  if (score_enabled) {
    apply_score_delta(kScoreCatch, score_anchor, true);
  }
  collected_counts[type] += 1;
  update_scoreboard_for(type);
  maybe_award_recipe_milestone(type, progress_for_type(*level, type), recipe_it->second, score_anchor,
                               score_enabled);
  if (progress_for_type(*level, type) > recipe_it->second) {
    trigger_failure(LossReason::TooMany, type);
    return;
  }
  check_completion();
}

inline void on_mushroom_missed(const std::string& type, ecs::Entity* entity) {
  if (!entity || entity->is_pending_deletion()) return;
  if (vfx::is_mushroom_vfx_locked(entity)) return;
  if (tutorial_miss_hook) {
    tutorial_miss_hook(type, entity);
  }
  if (!current_level()) return;
  auto active_it = active_entities.find(type);
  if (active_it != active_entities.end()) {
    active_it->second.erase(entity);
  }
  if (!tutorial_mode && !is_infinite_collector_run()) {
    apply_score_delta(kScoreMiss, score_anchor_for_entity(entity), true);
  }
  vfx::spawn_miss_effect(entity);
  camera_shake::add_trauma(0.06f);
  if (!tutorial_mode) {
    if (current_game_mode == GameMode::Collector) {
      decrement_collector_life();
      if (collector_lives_remaining <= 0) {
        trigger_failure(LossReason::Dropped, type);
        return;
      }
    }
  }
  check_completion();
}

inline void on_mushroom_sorted(ecs::Entity* entity) {
  if (!entity) return;
  if (vfx::is_mushroom_vfx_locked(entity)) return;
  auto* level = current_level();
  if (!level) {
    entity->mark_deleted();
    return;
  }
  auto* sprite = entity->get<render_system::SpriteRenderable>();
  std::string type = sprite ? engine::resources::texture_name(sprite->texture_id) : "";
  if (tutorial_sort_hook) {
    tutorial_sort_hook(type, entity);
  }
  shrooms::audio::play_mushroom_shot();
  auto active_it = active_entities.find(type);
  if (active_it != active_entities.end()) {
    active_it->second.erase(entity);
  }
  const glm::vec2 score_anchor = score_anchor_for_entity(entity);
  const bool score_enabled = !tutorial_mode;
  auto recipe_it = level->recipe.find(type);
  if (score_enabled) {
    apply_score_delta(kScoreSort, score_anchor, true);
  }
  sorted_counts[type] += 1;
  update_scoreboard_for(type);
  if (recipe_it != level->recipe.end()) {
    maybe_award_recipe_milestone(type, progress_for_type(*level, type), recipe_it->second,
                                 score_anchor, score_enabled);
  }
  if (recipe_it != level->recipe.end() && progress_for_type(*level, type) > recipe_it->second) {
    trigger_failure(LossReason::TooMany, type);
  }
  vfx::spawn_destroy_effect(entity);
  camera_shake::add_trauma(0.1f);
  entity->mark_deleted();
  check_completion();
}

inline void configure_spawners_for_level(const LevelDefinition& level) {
  disable_all_spawners();
  if (is_infinite_collector_run()) {
    reset_infinite_collector_queue();
    activate_infinite_collector_front_spawner();
    return;
  }
  for (const auto& plan : level.spawners) {
    auto it = spawners_by_type.find(plan.type);
    if (it == spawners_by_type.end()) {
      std::cerr << "No spawner registered for type " << plan.type << std::endl;
      continue;
    }
    auto* spawner = it->second;
    spawner->configure(plan.period, plan.density, plan.total_to_spawn);
    spawner->enabled = true;
  }
}

inline void start_level_with_definition(const LevelDefinition& level, size_t display_index,
                                        size_t seed_index, const std::string& status_label) {
  current_level_index = display_index;
  level_failed = false;
  level_finished = false;
  game_over_pending = false;
  pending_loss = LossInfo{};
  reset_active_entities();

  last_played_level_index = display_index;
  last_game_status = status_label;
  last_game_success = false;
  const bool reset_run_score = !infinite_mode || infinite_round_index == 0;
  if (reset_run_score) {
    current_run_score = 0;
    milestone_bonus_awarded.clear();
    scoreboard::set_score(0);
  } else {
    scoreboard::set_score(current_run_score);
  }
  collected_counts.clear();
  sorted_counts.clear();
  for (const auto& [type, target] : level.recipe_order) {
    collected_counts[type] = 0;
    sorted_counts[type] = 0;
  }
  for (const auto& plan : level.spawners) {
    collected_counts.try_emplace(plan.type, 0);
    sorted_counts.try_emplace(plan.type, 0);
  }
  const std::string score_task = "";
  scoreboard::init_with_targets(level.recipe_order, score_task);
  reset_collector_lives_if_needed();
  configure_spawners_for_level(level);
  seed_spawners_for_level(level, seed_index);
  for (const auto& [type, target] : level.recipe_order) {
    update_scoreboard_for(type);
  }
  apply_level_background(level);

  if (auto* main_scene = scene::get_scene("main")) {
    main_scene->activate();
    main_scene->set_pause(true);
  }
}

inline void start_infinite_mode() {
  refresh_daily_seed_if_needed();
  tutorial_mode = false;
  infinite_round_index = 0;
  infinite_rounds_won = 0;
  build_infinite_level(infinite_round_index);
  infinite_preview_date = current_daily_date;
  infinite_mode = true;
  infinite_preview_ready = false;
  current_run_score = 0;
  milestone_bonus_awarded.clear();
  if (current_game_mode == GameMode::Collector) {
    reset_infinite_collector_queue();
  }
  const std::string status =
      current_game_mode == GameMode::Collector
          ? "Infinite run (score " + std::to_string(current_run_score) + ")"
          : "Infinite run: round " + std::to_string(infinite_round_index + 1) +
                " (score " + std::to_string(current_run_score) + ")";
  start_level_with_definition(infinite_level, infinite_menu_index(),
                              static_cast<size_t>(infinite_round_index), status);
}

inline void advance_infinite_round() {
  if (current_game_mode == GameMode::Collector) return;
  apply_score_delta(kScoreRoundAdvance, scoreboard::score_anchor_px(), true);
  infinite_rounds_won += 1;
  infinite_round_index += 1;
  build_infinite_level(infinite_round_index);
  const std::string status = "Infinite run: round " + std::to_string(infinite_round_index + 1) +
                             " (score " + std::to_string(current_run_score) + ")";
  start_level_with_definition(infinite_level, infinite_menu_index(),
                              static_cast<size_t>(infinite_round_index), status);
}

inline void start_level(size_t index) {
  infinite_mode = false;
  tutorial_mode = false;
  infinite_preview_ready = false;
  infinite_preview_date.clear();
  if (parsed_levels.empty()) {
    std::cerr << "No levels parsed" << std::endl;
    return;
  }
  if (index >= parsed_levels.size()) {
    index = 0;
  }
  const auto& level = parsed_levels[index];
  const std::string status = "Playing " + level.id;
  start_level_with_definition(level, index, index, status);
}

inline void start_tutorial_mode() {
  infinite_mode = false;
  infinite_preview_ready = false;
  infinite_preview_date.clear();
  tutorial_mode = true;
  tutorial_level = LevelDefinition{};
  if (!parsed_levels.empty()) {
    tutorial_level.id = parsed_levels.front().id;
  } else {
    tutorial_level.id = "level_1_eli";
  }
  tutorial_level.objective_rule = ObjectiveRule::CollectOnly;
  tutorial_level.objective_hint.clear();
  const std::string status = "Playing tutorial";
  start_level_with_definition(tutorial_level, tutorial_menu_index(), 0, status);
}

inline void finish_tutorial(bool success) {
  if (!tutorial_mode) return;
  if (level_finished) return;
  if (success) {
    start_level_completed_transition();
    return;
  }
  level_failed = true;
  level_finished = true;
  finalize_level(false);
}

inline void restart_level() {
  if (infinite_mode) {
    infinite_round_index = 0;
    infinite_rounds_won = 0;
    infinite_preview_ready = false;
    infinite_preview_date.clear();
    start_infinite_mode();
    return;
  }
  if (tutorial_mode) {
    start_tutorial_mode();
    return;
  }
  start_level(current_level_index);
}

inline void advance_level() {
  if (infinite_mode) {
    advance_infinite_round();
    return;
  }
  if (tutorial_mode) {
    start_tutorial_mode();
    return;
  }
  size_t next_index = current_level_index + 1;
  if (next_index >= parsed_levels.size()) {
    next_index = 0;
  }
  start_level(next_index);
}

inline bool all_spawns_exhausted() {
  if (is_infinite_collector_run()) return false;
  auto* level = current_level();
  if (!level) return false;
  for (const auto& plan : level->spawners) {
    auto it = spawners_by_type.find(plan.type);
    if (it == spawners_by_type.end()) continue;
    auto* spawner = it->second;
    if (!spawner) continue;
    if (!spawner->is_depleted()) {
      return false;
    }
    if (!active_entities[plan.type].empty()) {
      return false;
    }
  }
  return true;
}

inline int remaining_spawns_for(const std::string& type) {
  auto it = spawners_by_type.find(type);
  if (it == spawners_by_type.end()) return 0;
  auto* spawner = it->second;
  if (!spawner) return 0;
  if (spawner->max_spawn_count < 0) {
    return -1;
  }
  return std::max(0, spawner->max_spawn_count - spawner->spawned_count);
}

inline LossInfo evaluate_loss_info() {
  LossInfo info{};
  if (tutorial_mode) return info;
  if (current_game_mode == GameMode::Collector) return info;
  auto* level = current_level();
  if (!level) return info;
  for (const auto& [type, target] : level->recipe_order) {
    const int progress = progress_for_type(*level, type);
    if (progress > target) {
      info.reason = LossReason::TooMany;
      info.type = type;
      return info;
    }
    int active_count = 0;
    auto active_it = active_entities.find(type);
    if (active_it != active_entities.end()) {
      active_count = static_cast<int>(active_it->second.size());
    }
    const int remaining = remaining_spawns_for(type);
    if (remaining < 0) {
      continue;
    }
    if (progress + active_count + remaining < target) {
      info.reason = LossReason::NotEnough;
      info.type = type;
      return info;
    }
  }
  return info;
}

inline void trigger_failure(LossReason reason, const std::string& type) {
  if (tutorial_mode) return;
  if (level_finished || game_over_pending) return;
  level_failed = true;
  game_over_pending = true;
  pending_loss.reason = reason;
  pending_loss.type = type;
  for (auto& [_, spawner] : spawners_by_type) {
    if (spawner) {
      spawner->enabled = false;
    }
  }
}

inline void finalize_level(bool success) {
  auto* level = current_level();
  if (!level) return;

  int total_sorted = 0;
  for (const auto& [type, sorted] : sorted_counts) {
    total_sorted += sorted;
  }

  int total_collected = 0;
  for (const auto& [type, collected] : collected_counts) {
    total_collected += collected;
  }

  last_result.success = success;
  last_result.collected = total_collected;
  last_result.sorted = total_sorted;
  last_result.global_score = current_run_score;
  last_result.level_index = current_level_index;
  last_result.level_id = level->id;
  last_result.infinite_mode = infinite_mode;
  last_result.tutorial_mode = tutorial_mode;
  last_result.game_mode = current_game_mode;
  last_result.rounds_won = infinite_rounds_won;
  last_result.round_index = infinite_round_index + 1;
  last_result_valid = true;
  if (success) {
    last_loss = LossInfo{};
  }

  if (infinite_mode) {
    last_result.level_index = infinite_menu_index();
    last_result.level_id = "Daily Infinity Mode";
    last_game_status = "Infinite run ended (score " + std::to_string(current_run_score) + ")";
    last_game_success = success;
  } else if (tutorial_mode) {
    if (success) {
      mark_tutorial_completed();
      set_game_mode(GameMode::Collector);
      save_progress();
    }
    last_result.level_index = tutorial_menu_index();
    last_result.level_id = "Tutorial";
    last_game_status = success ? "Completed Tutorial" : "Failed Tutorial";
    last_game_success = success;
  } else if (success) {
    last_game_status = "Completed " + level->id + " (collected " +
                       std::to_string(total_collected) + ", sorted " +
                       std::to_string(total_sorted) + ")";
    last_game_success = true;
    unlock_next_level(current_level_index);
  } else {
    last_game_status = "Failed " + level->id + " (collected " +
                       std::to_string(total_collected) + ", sorted " +
                       std::to_string(total_sorted) + ")";
    last_game_success = false;
  }

  for (auto& [type, spawner] : spawners_by_type) {
    if (spawner) {
      spawner->enabled = false;
    }
  }
  reset_active_entities();
  if (infinite_mode) {
    infinite_mode = false;
  }
  if (tutorial_mode) {
    tutorial_mode = false;
  }
}

inline void finalize_failure() {
  if (!game_over_pending) return;
  last_loss = pending_loss;
  pending_loss = LossInfo{};
  game_over_pending = false;
  level_failed = true;
  level_finished = true;
  finalize_level(false);
}

inline void fail_level(LossReason reason = LossReason::None, const std::string& type = "") {
  if (tutorial_mode) return;
  trigger_failure(reason, type);
}

inline void check_completion() {
  if (tutorial_mode) return;
  if (level_finished || game_over_pending) return;
  if (is_infinite_collector_run()) return;
  if (!all_spawns_exhausted()) return;

  auto* level = current_level();
  if (!level) return;

  bool success = !level_failed;
  for (const auto& [type, target] : level->recipe_order) {
    if (progress_for_type(*level, type) < target) {
      success = false;
      break;
    }
  }
  if (infinite_mode) {
    if (success) {
      if (current_game_mode == GameMode::Recipe && !level_intro::is_active()) {
        const int next_round = infinite_round_index + 2;
        level_intro::start_recipe_round_transition(
            "Recipe complete",
            "Round " + std::to_string(next_round),
            []() { advance_infinite_round(); });
      }
      return;
    }
    LossInfo info = evaluate_loss_info();
    if (info.reason == LossReason::None) {
      info.reason = LossReason::NotEnough;
    }
    trigger_failure(info.reason, info.type);
    return;
  }
  if (success) {
    start_level_completed_transition();
    return;
  }
  LossInfo info = evaluate_loss_info();
  if (info.reason == LossReason::None) {
    info.reason = LossReason::NotEnough;
  }
  trigger_failure(info.reason, info.type);
}

inline void initialize() {
  current_game_mode = load_selected_mode();
  parse_levels(shrooms::asset_path("levels.data"));
  build_infinite_spawner_cache();
  leaderboard::set_profile(leaderboard_profile_for_mode(current_game_mode));
  leaderboard::load_or_default();
  current_daily_date.clear();
  current_daily_seed = 0;
  refresh_daily_seed_if_needed();
  level_finished = true;
  level_failed = false;
  game_over_pending = false;
  pending_loss = LossInfo{};
  last_loss = LossInfo{};
  current_level_index = 0;
  last_result_valid = false;
  last_result = LastResult{};
  infinite_mode = false;
  tutorial_mode = false;
  infinite_round_index = 0;
  infinite_rounds_won = 0;
  collector_lives_remaining = kCollectorLivesPerRun;
  current_run_score = 0;
  milestone_bonus_awarded.clear();
  infinite_preview_ready = false;
  infinite_preview_date.clear();
  infinite_level = LevelDefinition{};
  tutorial_level = LevelDefinition{};
  infinite_collector_queue.clear();
  infinite_collector_ticket_index = 0;
  infinite_collector_type_bag.clear();
  infinite_collector_previous_type_bag.clear();
  infinite_collector_type_bag_pos = 0;
  infinite_collector_type_bag_index = 0;
  progress_save_exists = false;
  tutorial_completed = false;
  load_progress();
  active_entities.clear();
  collected_counts.clear();
  sorted_counts.clear();
  if (parsed_levels.empty()) {
    last_game_status = "No levels configured";
  } else {
    last_game_status = "Select a level to begin";
  }
  last_game_success = false;
}

}  // namespace levels
