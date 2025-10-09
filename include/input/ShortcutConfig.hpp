#pragma once
#include <string>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>

namespace input {

class ShortcutConfig {
public:
    using KeySequence = std::vector<unsigned int>;
    using ShortcutMap = std::map<std::string, KeySequence>;
    
    ShortcutConfig();
    
    bool loadFromFile(const std::string& filepath);
    bool saveToFile(const std::string& filepath) const;
    
    void setShortcut(const std::string& action, const KeySequence& keys);
    const KeySequence& getShortcut(const std::string& action) const;
    const ShortcutMap& getAllShortcuts() const;
    
    void resetToDefaults();
    
    static std::string keySequenceToString(const KeySequence& keys);
    static KeySequence stringToKeySequence(const std::string& str);

private:
    void loadDefaultShortcuts();
    
    ShortcutMap m_shortcuts;
    static const inline std::string CONFIG_FILENAME = "shortcuts.json";
};

} // namespace input
