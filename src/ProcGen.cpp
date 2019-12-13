#include "ProcGen.h"
#include "Assets.h"
#include <stdlib.h>

Map ProcGen::generate_map(ProcGen::Rules *rules, V2 *dimensions)
{
    Map map;
    map.entity_manager = ECS::Manager();
    // Initialize map.
    for (int i = 0; i < dimensions->x; ++i)
    {
        std::vector<Cell> column;
        for (int j = 0; j < dimensions->y; ++j)
        {
            column.push_back({-1});
        }
        map.grid.push_back(column);
    }
    // We are only placing trees right now:
    int num_trees = rand() % rules->tree_weight;
    int num_ground = rand() % rules->ground_weight;

    // Instantiate
    std::vector<int> trees;
    std::vector<int> ground;
    int texture_index = Assets::get_texture_index("tilesheet-transparent");
    for (int i = 0; i < num_trees; ++i)
    {
        ECS::Entity tree_entity;
        ECS::Component render_component;
        render_component.type = ECS::Type::RENDER;
        render_component.data.r = {
            {0, 17, 16, 16},
            Render::Layer::WORLD_LAYER,
            texture_index,
            1,
            1,
            true};
        tree_entity.components[render_component.type] = render_component;
        map.entity_manager.entities.push_back(tree_entity);
        trees.push_back(map.entity_manager.entities.size() - 1);
    }
    for (int i = 0; i < num_ground; ++i)
    {
        ECS::Entity ground_entity;
        ECS::Component render_component;
        render_component.type = ECS::Type::RENDER;
        render_component.data.r = {
            {17, 0, 16, 16},
            Render::Layer::WORLD_LAYER,
            texture_index,
            1,
            1,
            true};
        ground_entity.components[render_component.type] = render_component;
        map.entity_manager.entities.push_back(ground_entity);
        ground.push_back(map.entity_manager.entities.size() - 1);
    }

    // Placement
    for (unsigned int i = 0; i < trees.size(); ++i)
    {
        bool placed = false;
        while (!placed)
        {
            int x = rand() % dimensions->x;
            int y = rand() % dimensions->y;
            if (map.grid[x][y].entity_id == -1)
            {
                ECS::Entity *entity = &map.entity_manager.entities[trees[i]];
                ECS::Component position;
                position.type = ECS::Type::POSITION;
                position.data.p.position = {x * 16, y * 16};
                position.data.p.target_position = {x * 16, y * 16};
                entity->components[position.type] = position;
                entity->components[ECS::Type::DUMB_AI_COMPONENT] = {};
                map.grid[x][y].entity_id = trees[i];
                placed = true;
            }
        }
    }
    for (unsigned int i = 0; i < ground.size(); ++i)
    {
        bool placed = false;
        while (!placed)
        {
            int x = rand() % dimensions->x;
            int y = rand() % dimensions->y;
            if (map.grid[x][y].entity_id == -1)
            {
                ECS::Entity *entity = &map.entity_manager.entities[ground[i]];
                ECS::Component position;
                position.type = ECS::Type::POSITION;
                position.data.p.position = {x * 16, y * 16};
                position.data.p.target_position = {x * 16, y * 16};
                entity->components[position.type] = position;
                map.grid[x][y].entity_id = ground[i];
                placed = true;
            }
        }
    }
    // Place player
    bool player_placed = false;
    while (!player_placed)
    {
        int x = rand() % dimensions->x;
        int y = rand() % dimensions->y;
        if (map.grid[x][y].entity_id == -1)
        {
            ECS::Entity player;
            ECS::Component position;
            position.type = ECS::Type::POSITION;
            position.data.p.position = {x * 16, y * 16};
            position.data.p.target_position = {x * 16, y * 16};
            player.components[position.type] = position;
            player.components[ECS::Type::CAMERA] = {};
            player.components[ECS::Type::PLAYER_INPUT] = {};
            map.entity_manager.entities.push_back(player);
            map.grid[x][y].entity_id = map.entity_manager.entities.size() - 1;
            player_placed = true;
        }
    }
    for (int i = 0; i < 100; ++i)
    {
        bool npc_placed = false;
        while (!npc_placed)
        {
            int x = rand() % dimensions->x;
            int y = rand() % dimensions->y;
            if (map.grid[x][y].entity_id == -1)
            {
                ECS::Entity npc;
                ECS::Component position;
                position.type = ECS::Type::POSITION;
                position.data.p.position = {x * 16, y * 16};
                position.data.p.target_position = {x * 16, y * 16};
                npc.components[position.type] = position;
                ECS::Component render;
                render.type = ECS::Type::RENDER;
                render.data.r = {{425, 0, 16, 16},
                                 Render::Layer::WORLD_LAYER,
                                 texture_index,
                                 1,
                                 2,
                                 true};
                npc.components[render.type] = render;
                // npc.components[ECS::Type::DUMB_AI_COMPONENT] = {};
                map.entity_manager.entities.push_back(npc);
                map.grid[x][y].entity_id = map.entity_manager.entities.size() - 1;
                npc_placed = true;
            }
        }
    }
    return map;
}