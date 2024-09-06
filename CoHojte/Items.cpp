#include "Items.h"

#include "dma.h"
#include "Utility.h"

void printBufferAsHex(const char* buffer, size_t length)
{
    for (size_t i = 0; i < length; ++i)
    {
        printf("%02X", (unsigned char)buffer[i]);
    }
}

std::vector<std::string> ignoreItems = {"stem_card", "_zm_", "furniture", "tool_box_metal", "school_locker"};

DWORD WINAPI ItemThread(void* params)
{
    while (g_Active)
    {
        if (g_decryptBasePtr == 0 || !g_ItemESP)
        {
            Sleep(6000);
            continue;
        }
        constexpr size_t ID_BUFFER_SIZE = 48;
        int validItems = 0;
        std::vector<item_s> closeItems = std::vector<item_s>(7, item_s());

        // BULK READ
        Loot_t* lootArray = new Loot_t[MAX_ITEM_COUNT];
        DMA::read_memory(g_Base + Offsets::LOOT_PTR, reinterpret_cast<uint64_t>(lootArray), sizeof(Loot_t) * MAX_ITEM_COUNT);

        for (int i = 0; i < MAX_ITEM_COUNT; i++)
        {
            item_s currItem = item_s();
            currItem.isValid = 1 == lootArray[i].valid;
            if (!currItem.isValid)
            {
                g_ItemList[i] = item_s();
                continue;
            }
            currItem.isOpenable = lootArray[i].isOpenable;
            currItem.pos = lootArray[i].pos;
            if (currItem.pos.x == 0 || currItem.pos.y == 0)
            {
                g_ItemList[i] = item_s();
                continue;
            }
            currItem.pos.z -= 45; // adjustment
            validItems++;

            // only update name when new position or no name
            if (g_ItemList[i].pos == currItem.pos && !g_ItemList[i].friendlyName.empty())
                currItem.friendlyName = g_ItemList[i].friendlyName; // Use old name / cached name
            else
            {
                uint64_t namePtr2 = DMA::Read<uint64_t>(lootArray[i].namePtr + 0x8);
                currItem.Id = new char[ID_BUFFER_SIZE];
                DMA::read_memory(namePtr2, reinterpret_cast<uint64_t>(currItem.Id), ID_BUFFER_SIZE);
                currItem.Id[ID_BUFFER_SIZE - 1] = '\0'; // null terminate
                currItem.friendlyName = std::string(currItem.Id);
            }
            if(currItem.friendlyName.empty())
            {
                continue;
            }
            // Item name and search
            // Check if item name matches search list
            currItem.isSearched = false;
            for (const auto& keyword : g_ItemSearchKeywords)
            {
                if (stringToLower(currItem.friendlyName).find(stringToLower(keyword)) != std::string::npos)
                {
                    currItem.isSearched = true;
                    break; // Found a keyword match, no need to check other keywords for this item
                }
            }

            // Check if item name matches ignore list
            for (const auto& ignoreWord : ignoreItems)
            {
                if (currItem.friendlyName.find(ignoreWord) != std::string::npos)
                {
                    currItem.isValid = false;
                    break; // Found a keyword match, no need to check other keywords for this item
                }
            }
            if (!currItem.isSearched && !currItem.isValid) // filter out the item
            {
                g_ItemList[i] = item_s();
                continue;
            }

            currItem.idx = i;
            currItem.distToLocal = std::round(g_lPlayer.pos.distance(currItem.pos));
            
            
            if (g_Verbose) // Find the position where currItem should be inserted to keep closeItems sorted
            {
                auto dist = currItem.distToLocal;
                auto it = std::find_if(closeItems.begin(), closeItems.end(), [dist](const item_s& item)
                {
                    return dist < g_lPlayer.pos.distance(item.pos);
                });
                closeItems.insert(it, currItem);
                if (closeItems.size() > 7) closeItems.pop_back();
            }

            // OK Item
            std::regex reContainer(make_string("container|military|uranium").c_str()); // todo: add more/check container keywords
            // check if item is a container
            if (std::regex_search(currItem.friendlyName, reContainer)) currItem.isContainer = true;
            currItem.friendlyName = makeFriendlyName(currItem.friendlyName);

            g_ItemList[i] = currItem;
        }
        if (g_Verbose)
        {
            // Sort items by distance to player
            std::sort(closeItems.begin(), closeItems.end(), [](const item_s& a, const item_s& b)
            {
                return a.pos.distance(g_lPlayer.pos) < b.pos.distance(g_lPlayer.pos);
            });
            // Print close items for debugging
            for (auto& item : closeItems)
            {
                //printBufferAsHex(item.Id, ID_BUFFER_SIZE);
                printf("#%d: %s, Pos: %0f, %0f, %0f\n", item.idx, item.friendlyName.c_str(), item.pos.x, item.pos.y,
                       item.pos.z);
            }
            printf("Valid items: %d List: %lld\n", validItems, closeItems.size());
            Sleep(3000);
        }
        Sleep(1000); // Update once every second
    }
    return 0;
}

std::string makeFriendlyName(const std::string& itemName) {
    // Step 1: Split by underscores or "::"
    std::vector<std::string> parts;
    std::regex regex_split("[_::]+");  // Split on underscore or "::"
    std::sregex_token_iterator it(itemName.begin(), itemName.end(), regex_split, -1);
    std::sregex_token_iterator reg_end;

    for (; it != reg_end; ++it) {
        if (!it->str().empty()) {
            parts.push_back(it->str());
        }
    }

    // Step 2: Remove unwanted parts
    std::vector<std::string> filtered;
    // Regex to skip alphanumeric (letters followed by numbers or vice versa) and purely numeric parts,
    // but keep 'rarity' followed by a number
    std::regex unwanted_regex(make_string("^((?!rarity[0-9]+)[a-zA-Z]*[0-9]+[a-zA-Z]*|[0-9]+)$"));
    std::regex unwanted_keywords(make_string("^(lm|wm|br|un|zm|vm|bp|ee|uk|jup|rig|rust|loot|offhand|blue|military|accessory|props?|electronics|store|utility|construction|carbon|fiber|item|pickup|bathroom|stem|card)$"), std::regex_constants::icase);
    for (const auto& part : parts) {
        if (std::regex_match(part, unwanted_regex) || std::regex_match(part, unwanted_keywords)) {
            continue;  // Skip unwanted parts
        }
        filtered.push_back(part);
    }

    // Step 3: Join remaining parts with spaces
    std::ostringstream oss;
    if (!filtered.empty()) {
        std::copy(filtered.begin(), filtered.end() - 1, std::ostream_iterator<std::string>(oss, " "));
        oss << filtered.back();  // Append last element without extra space
    }

    // Step 4: Capitalize the first letter of each word (optional)
    std::string result = oss.str();
    if (!result.empty()) {
        result[0] = std::toupper(result[0]);
        for (size_t i = 1; i < result.length(); ++i) {
            if (result[i - 1] == ' ') {
                result[i] = std::toupper(result[i]);
            }
        }
    }

    return result;
}
