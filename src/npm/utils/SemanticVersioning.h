#pragma once

#include <string_view>
#include <charconv>
#include <tuple>
#include <array>
#include <functional>

using namespace std::string_view_literals;

namespace sylvanmats::npm::utils{

// 1. Core structural types
struct SubToken {
    std::string_view raw{};
    bool is_numeric = false;
    int numeric_value = 0;

    static SubToken parse(std::string_view token) noexcept {
        SubToken st{token, true, 0};
        if (token.empty()) { st.is_numeric = false; return st; }
        
        auto res = std::from_chars(token.data(), token.data() + token.size(), st.numeric_value);
        if (res.ec != std::errc{} || res.ptr != token.data() + token.size()) {
            st.is_numeric = false; 
        }
        return st;
    }
};

inline std::string_view pop_next_sub_token(std::string_view& src) noexcept {
    if (src.empty()) return {};
    size_t dot = src.find('.');
    if (dot == std::string_view::npos) {
        std::string_view ret = src;
        src = {};
        return ret;
    }
    std::string_view ret = src.substr(0, dot);
    src.remove_prefix(dot + 1);
    return ret;
}

inline bool compare_prerelease(std::string_view lhs_raw, std::string_view rhs_raw) noexcept {
    if (lhs_raw.empty() && !rhs_raw.empty()) return false; 
    if (!lhs_raw.empty() && rhs_raw.empty()) return true;  
    if (lhs_raw.empty() && rhs_raw.empty()) return false;

    std::string_view lhs = lhs_raw;
    std::string_view rhs = rhs_raw;

    while (!lhs.empty() || !rhs.empty()) {
        std::string_view l_str = pop_next_sub_token(lhs);
        std::string_view r_str = pop_next_sub_token(rhs);

        if (l_str.empty() && !r_str.empty()) return true;
        if (!l_str.empty() && r_str.empty()) return false;

        SubToken l_tok = SubToken::parse(l_str);
        SubToken r_tok = SubToken::parse(r_str);

        if (l_tok.is_numeric && r_tok.is_numeric) {
            if (l_tok.numeric_value != r_tok.numeric_value) {
                return l_tok.numeric_value < r_tok.numeric_value;
            }
        } 
        else if (!l_tok.is_numeric && !r_tok.is_numeric) {
            if (l_tok.raw != r_tok.raw) {
                return l_tok.raw < r_tok.raw;
            }
        } 
        else {
            return l_tok.is_numeric; 
        }
    }
    return false;
}

struct SemVer {
    int major = -1;
    int minor = -1;
    int patch = -1;
    std::string_view prerelease{};
    std::string_view build{}; // Preserved, but ignored in precedence checks

    bool is_valid() const noexcept { return major >= 0; }

    bool operator<(const SemVer& o) const noexcept {
        if (major != o.major) return major < o.major;
        if (minor != o.minor) return minor < o.minor;
        if (patch != o.patch) return patch < o.patch;
        return compare_prerelease(prerelease, o.prerelease);
    }

    bool operator==(const SemVer& o) const noexcept {
        return major == o.major && minor == o.minor && patch == o.patch && prerelease == o.prerelease;
    }
    bool operator>=(const SemVer& o) const noexcept { return !(*this < o); }
    bool operator<=(const SemVer& o) const noexcept { return !(o < *this); }
    bool operator>(const SemVer& o) const noexcept { return o < *this; }
};

// 2. Tokenizers and String Handlers
constexpr std::string_view trim(std::string_view str) noexcept {
    while (!str.empty() && (str.front() == ' ' || str.front() == '\t')) str.remove_prefix(1);
    while (!str.empty() && (str.back() == ' ' || str.back() == '\t')) str.remove_suffix(1);
    return str;
}

inline std::string_view split_next(std::string_view& src, std::string_view token) noexcept {
    if (src.empty()) return {};
    size_t pos = src.find(token);
    if (pos == std::string_view::npos) {
        std::string_view ret = src;
        src = {};
        return trim(ret);
    }
    std::string_view ret = src.substr(0, pos);
    src.remove_prefix(pos + token.size());
    return trim(ret);
}

inline SemVer parse_version(std::string_view v) noexcept {
    SemVer sv;
    v = trim(v);
    if (v.empty()) return sv;
    if (v.front() == 'v' || v.front() == 'V') v.remove_prefix(1);
    if (!v.empty() && v.front() == '=') v.remove_prefix(1);

    size_t plus = v.find('+');
    if (plus != std::string_view::npos) {
        sv.build = v.substr(plus + 1);
        v = v.substr(0, plus);
    }
    size_t dash = v.find('-');
    if (dash != std::string_view::npos) {
        sv.prerelease = v.substr(dash + 1);
        v = v.substr(0, dash);
    }

    const char* ptr = v.data();
    const char* end = v.data() + v.size();

    if (ptr < end && (*ptr == 'x' || *ptr == 'X' || *ptr == '*')) { sv.major = 999; return sv; }
    auto res = std::from_chars(ptr, end, sv.major);
    if (res.ec != std::errc{} || res.ptr == end || *res.ptr != '.') return sv;
    ptr = res.ptr + 1;

    if (ptr < end && (*ptr == 'x' || *ptr == 'X' || *ptr == '*')) { sv.minor = 999; return sv; }
    res = std::from_chars(ptr, end, sv.minor);
    if (res.ec != std::errc{} || res.ptr == end || *res.ptr != '.') return sv;
    ptr = res.ptr + 1;

    if (ptr < end && (*ptr == 'x' || *ptr == 'X' || *ptr == '*')) { sv.patch = 999; return sv; }
    std::from_chars(ptr, end, sv.patch);

    return sv;
}

// 3. Range-Boundary Mathematical Modeling
struct VersionRange {
    SemVer min_version{}; 
    SemVer max_version{}; 
    bool has_max = false;

    bool satisfies(const SemVer& target) const noexcept {
        if (target < min_version) return false;
        if (has_max && target >= max_version) return false;
        
        if (!target.prerelease.empty()) {
            if (min_version.prerelease.empty() || 
                target.major != min_version.major || 
                target.minor != min_version.minor || 
                target.patch != min_version.patch) {
                return false;
            }
        }
        return true;
    }
};

inline VersionRange evaluate_range_bounds(std::string_view op, SemVer v) noexcept {
    VersionRange range;
    range.min_version = v;

    if (op == "=" || op.empty()) {
        range.max_version = {v.major, v.minor, v.patch + 1};
        range.has_max = true;
    }
    else if (op == ">=") {
        range.has_max = false; 
    }
    else if (op == ">") {
        range.min_version = {v.major, v.minor, v.patch + 1};
        range.has_max = false;
    }
    else if (op == "<=") {
        range.min_version = {0, 0, 0};
        range.max_version = {v.major, v.minor, v.patch + 1}; 
        range.has_max = true;
    }
    else if (op == "<") {
        range.min_version = {0, 0, 0};
        range.max_version = v;
        range.has_max = true;
    }
    else if (op == "~") {
        range.has_max = true;
        if (v.minor == -1 || v.minor == 999) { 
            range.min_version = {v.major, 0, 0};
            range.max_version = {v.major + 1, 0, 0};
        } else { 
            range.max_version = {v.major, v.minor + 1, 0};
        }
    }
    else if (op == "^") {
        range.has_max = true;
        if (v.major > 0) { 
            range.max_version = {v.major + 1, 0, 0};
        } 
        else if (v.minor > 0) { 
            range.max_version = {0, v.minor + 1, 0};
        } 
        else { 
            range.max_version = {0, 0, v.patch + 1};
        }
    }

    if (range.min_version.minor == -1 || range.min_version.minor == 999) range.min_version.minor = 0;
    if (range.min_version.patch == -1 || range.min_version.patch == 999) range.min_version.patch = 0;

    return range;
}

    class SemanticVersioning{
    protected:
        bool update;
    public:
        SemanticVersioning(bool update=false) : update (update) {};
        SemanticVersioning(const SemanticVersioning* orig) = delete;
        virtual ~SemanticVersioning() = default;
    public:
        bool operator()(std::string_view val, std::function<void(std::string_view base, std::string_view branch, std::string_view wildcard)> apply){

            
            if (size_t hash_pos = val.find('#');hash_pos != std::string_view::npos) {
                apply(val.substr(0, hash_pos), val.substr(hash_pos + 1), ""sv);
                return false;
            }
            
            VersionRange constraint = evaluate_range_bounds("^", parse_version(val));

            std::string_view best_match_version{};
            SemVer highest_resolved{-1, -1, -1};

            // Loop through string_views returned by json-thresher over the manifest's version keys
            std::vector<std::string_view> version_keys_from_thresher{{val}};
            for (std::string_view available_version : version_keys_from_thresher) {
                SemVer candidate = parse_version(available_version);
                
                if (constraint.satisfies(candidate)) {
                    if (candidate > highest_resolved) {
                        highest_resolved = candidate;
                        best_match_version = available_version; // Save zero-allocation reference
                    }
                }
            }
            if (best_match_version.empty()) return false;
            // Result is processed with zero allocations and zero regex state evaluations!
            std::cout << "Highest resolved package coordinate matching range: " << best_match_version << "\n";
            std::string_view wildcard{highest_resolved.prerelease};
            apply(best_match_version, ""sv, wildcard);

//             std::regex versionExpression(R"((\^|\~)?((\d+)(\.(\d+|x)(\.(\d+|x))?)?))");
//             std::smatch versionMatch;
//             std::string semanticVersion=std::string{val};
//             if (std::regex_match(semanticVersion, versionMatch, versionExpression)) {
//                 std::string version;
//                 std::string wildcard="latest";
//                 std::string major;
//                 std::string minor;
//                 std::string patch;
//                 std::string prefix;
// //                std::cout<<"versionMatch? "<<versionMatch.size()<<std::endl;
//                 if (versionMatch.size() >= 4) {
//                     std::ssub_match base_sub_match = versionMatch[3];
//                     major = base_sub_match.str();
//                 }
//                 if (versionMatch.size() >= 6) {
//                     std::ssub_match base_sub_match = versionMatch[5];
//                     minor = base_sub_match.str();
//                 }
//                 if (versionMatch.size() >= 8) {
//                     std::ssub_match base_sub_match = versionMatch[7];
//                     patch = base_sub_match.str();
//                 }
//                 if (versionMatch.size() >= 3) {
//                     std::ssub_match base_sub_match = versionMatch[2];
//                     version=base_sub_match.str();
//                     wildcard=version;
// //                    std::cout << val << " has is " << version << '\n';
//                 }
//                 if (versionMatch.size() >= 2) {
//                     std::ssub_match base_sub_match = versionMatch[1];
//                     prefix = base_sub_match.str();
//                 }
//                 if(update && prefix.compare("^")==0)patch="x";
//                 wildcard=major+"."+minor+"."+patch;
// //                std::cout << val << " becomes " << wildcard <<" "<<major<<" "<<minor<<" "<<patch<<" "<< '\n';
//                 apply(std::string_view{version}, std::string_view{wildcard});
//                 return true;
//             }
             return true;
        }
    };

}
