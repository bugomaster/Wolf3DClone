#include "pch.hpp"
#include "LevelData.hpp"
std::vector<std::string> splitByString(const std::string& input, const std::string& delimiter, size_t keepSize = 0)
{
    std::vector<std::string> result;
    if (input == "")
        return result;

    if (delimiter.empty())
    {
        result.push_back(input);
        return result;
    }


    size_t start = 0;
    size_t pos = 0;

    while ((pos = input.find(delimiter, start)) != std::string::npos)
    {
        result.emplace_back(input.substr(start, pos - start));
        start = pos + delimiter.length();
    }

    result.emplace_back(input.substr(start));

    std::string bottomAddStr = "";
    for (size_t i = 0; i < keepSize; i++)
    {
        bottomAddStr += delimiter.at(i);
    }
    for (auto& part : result)
    {
        part = part + bottomAddStr;
    }
    return result;
}
std::string trim(const std::string& str)
{
    size_t first = str.find_first_not_of(" \t\r\n");

    if (first == std::string::npos)
        return "";

    size_t last = str.find_last_not_of(" \t\r\n");

    return str.substr(first, last - first + 1);
}
Vector2f parseVector2(const std::string& str)
{
    Vector2f result{};

    sscanf_s(
        str.c_str(),
        "(%f,%f)",
        &result.x,
        &result.y
    );

    return result;
}

std::vector<std::vector<int>> loadMap(const std::string& filename)
{
    std::vector<std::vector<int>> map;

    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Failed to open " << filename << '\n';
        return map;
    }

    std::string line;

    while (std::getline(file, line)) {
        if (line.empty())
            continue;

        std::vector<int> row;
        row.reserve(line.size());

        for (char c : line) {
            if (c == '0' || c == '1') {
                row.push_back(c - '0');
            }
        }

        if (!row.empty())
            map.push_back(std::move(row));
    }

    return map;
}



bool LevelData::loadLevelProperties(const std::string& path) {
    this->map = loadMap(path + "map.txt");

    std::ifstream file(path + "level.levelproperties");
    if (!file.is_open())
    {
        std::cerr << "Failed to open level file\n";
        return false;
    }

    std::string line;
    while (std::getline(file, line))
    {
        line = trim(line);
        if (line.empty())
            continue;
        if (line[0] == '#')
            continue;



        while (!line.empty() && line.back() == '\\')
        {
            line.pop_back();

            std::string nextLine;

            if (std::getline(file, nextLine))
            {
                line += trim(nextLine);
            }
        }

        size_t eq = line.find('=');

        if (eq == std::string::npos)
            continue;

        std::string key = trim(line.substr(0, eq));
        std::string value = trim(line.substr(eq + 1));

        if (key == "PLAYER_START")
            this->playerStart = parseVector2(value);
        else if (key == "CAMERA_START")
            this->cameraStart = parseVector2(value);
        else if (key == "SECRET_WALLS")
        {

            std::string entry;
            auto parts = splitByString(value, ";,", 0);

            for (auto part : parts)
            {
                entry = part;
                entry = trim(entry);

                if (entry.empty())
                    continue;

                SecretWall wall{};

                sscanf_s(
                    entry.c_str(),
                    "(%d,%d) (%d,%d) %d",
                    &wall.position.x,
                    &wall.position.y,
                    &wall.dirMove.x,
                    &wall.dirMove.y,
                    &wall.tileID
                );
                this->secretWalls.push_back(wall);
            }
        }
        else if (key == "LOCK_GATES")
        {

            std::string entry;
            auto parts = splitByString(value, ";,", 0);

            for (auto part : parts)
            {
                entry = part;
                entry = trim(entry);

                if (entry.empty())
                    continue;

                LockGate lockGate{};

                sscanf_s(
                    entry.c_str(),
                    "(%d,%d) %d",
                    &lockGate.position.x,
                    &lockGate.position.y,
                    &lockGate.keyID
                );
                this->lockGates.push_back(lockGate);
            }
        }
        else if (key == "DOORS")
        {

            std::string entry;
            auto parts = splitByString(value, ";,", 0);

            for (auto part : parts)
            {
                entry = part;
                entry = trim(entry);

                if (entry.empty())
                    continue;

                Door door{};

                sscanf_s(
                    entry.c_str(),
                    "(%d,%d) (%d,%d)",
                    &door.position.x,
                    &door.position.y,
                    &door.dirMove.x,
                    &door.dirMove.y
                );
                this->doors.push_back(door);
            }
        }
        else if (key == "END_GATE")
        {

            std::string entry;
            auto parts = splitByString(value, ";,", 0);

            for (auto part : parts)
            {
                entry = part;
                entry = trim(entry);

                if (entry.empty())
                    continue;

                Vector2i endGate{};

                sscanf_s(
                    entry.c_str(),
                    "(%d,%d)",
                    &endGate.x,
                    &endGate.y
                );
                this->endGate = endGate;
            }
        }
        else if (key == "COLLECTBLES")
        {

            std::string entry;
            auto parts = splitByString(value, ";,", 0);

            for (auto part : parts)
            {
                entry = part;
                entry = trim(entry);

                if (entry.empty())
                    continue;

                CollectibleData collectble{};
                char tag[32]{};

                sscanf_s(
                    entry.c_str(),
                    "(%f,%f) %31s",
                    &collectble.position.x,
                    &collectble.position.y,
                    tag,
                    (unsigned)_countof(tag)
                );


                if (strcmp(tag, "TROPHIE") == 0)
                    collectble.type = Collectible::TROPHIE;
                else if (strcmp(tag, "GOLDBOX") == 0)
                    collectble.type = Collectible::GOLDBOX;
                else if (strcmp(tag, "AMMO") == 0)
                    collectble.type = Collectible::AMMO;
                else if (strcmp(tag, "MEAL") == 0)
                    collectble.type = Collectible::MEAL;
                else if (strcmp(tag, "MEATBALLS") == 0)
                    collectble.type = Collectible::MEATBALLS;


                this->collectibles.push_back(collectble);
            }
        }
        else if (key == "KEYS")
        {

            std::string entry;
            auto parts = splitByString(value, ";,", 0);

            for (auto part : parts)
            {
                entry = part;
                entry = trim(entry);

                if (entry.empty())
                    continue;

                KeyData key{};
                char tag[32]{};

                sscanf_s(
                    entry.c_str(),
                    "(%f,%f) %d",
                    &key.position.x,
                    &key.position.y,
                    &key.keyID
                );

                this->keys.push_back(key);
            }
        }
        else if (key == "DECORATIONS")
        {

            std::string entry;
            auto parts = splitByString(value, ";,", 0);

            for (auto part : parts)
            {
                entry = part;
                entry = trim(entry);

                if (entry.empty())
                    continue;

                DecorationData decoration{};
                char type[32]{};

                sscanf_s(
                    entry.c_str(),
                    "(%f,%f) %31s",
                    &decoration.position.x,
                    &decoration.position.y,
                    type,
                    (unsigned)_countof(type)

                );
                if (strcmp(type, "LAMP") == 0)
                    decoration.type = Decoration::LAMP;
                else if (strcmp(type, "TREE") == 0)
                    decoration.type = Decoration::TREE;
                else if (strcmp(type, "FLAG") == 0)
                    decoration.type = Decoration::FLAG;
                else if (strcmp(type, "TABLE") == 0)
                    decoration.type = Decoration::TABLE;


                this->decorations.push_back(decoration);
            }
        }
        else if (key == "GUARDS")
        {

            std::string entry;
            auto parts = splitByString(value, ";,", 0);

            for (auto part : parts)
            {
                entry = part;
                entry = trim(entry);

                if (entry.empty())
                    continue;

                Vector2f pos;
                sscanf_s(
                    entry.c_str(),
                    "(%f,%f)",
                    &pos.x,
                    &pos.y
                );

                this->guards.push_back(pos);
            }
        }
        else if (key == "HOUNDS")
        {

            std::string entry;
            auto parts = splitByString(value, ";,", 0);

            for (auto part : parts)
            {
                entry = part;
                entry = trim(entry);

                if (entry.empty())
                    continue;

                Vector2f pos;
                sscanf_s(
                    entry.c_str(),
                    "(%f,%f)",
                    &pos.x,
                    &pos.y
                );

                this->hounds.push_back(pos);
            }
        }
        else if (key == "rr")
        {
            std::string entry;
            auto parts = splitByString(value, ";,", 0);

            for (auto part : parts)
            {
                entry = part;
                entry = trim(entry);

                if (entry.empty())
                    continue;

                WeaponData data;
                char type[32]{};
                sscanf_s(
                    entry.c_str(),
                    "(%f,%f) %31s",
                    &data.position.x,
                    &data.position.y,
                    type,
                    (unsigned)_countof(type)
                );
                if (strcmp("MACHINEGUN", type) == 0)
                {
                    data.type = Collectible::MACHINE_GUN;
                }
                else if (strcmp("RIFLE", type) == 0)
                {
                    data.type = Collectible::RIFLE;
                }
               this->weapons.push_back(data);
            }

        }

    }

    return true;
}
