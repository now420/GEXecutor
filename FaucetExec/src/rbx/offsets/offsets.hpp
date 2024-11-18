#include <cstdint>

namespace rbx {
    namespace offsets {
        namespace script {
            constexpr std::uint64_t msbytecode = 0x168;
            constexpr std::uint64_t lsbytecode = 0x1C0;

            constexpr std::uint64_t moduleflags = 0x1b0 - 0x4;
            constexpr std::uint64_t iscore = 0x1b0;
        }

        namespace datamodel {
            constexpr std::uint64_t dm1 = 0x118;
            constexpr std::uint64_t dm2 = 0x1a8;
            constexpr std::uint64_t placeid = 0x170;
            constexpr std::uint64_t gameloaded = 0x3B1;
        };

        namespace visualengine {
            constexpr std::uint64_t engine = 0x10;
            constexpr std::uint64_t viewmatrix = 0x4C0;
            constexpr std::uint64_t dimensions = 0x730;
        };

        namespace instance {
            constexpr std::uint64_t childsize = 0x8;
            constexpr std::uint64_t children = 0x70;
            constexpr std::uint64_t parent = 0x40;
            constexpr std::uint64_t name = 0x68;
            constexpr std::uint64_t cname = 0x8;
            constexpr std::uint64_t cdescriptor = 0x18;
            constexpr std::uint64_t primitive = 0x160;

            namespace basepart {
                constexpr std::uint64_t position = 0x140;
                constexpr std::uint64_t cframe = 0x11C;
                constexpr std::uint64_t size = 0x2AC;
                constexpr std::uint64_t velocity = 0x14C;
                constexpr std::uint64_t rotvelocity = 0x158;
                constexpr std::uint64_t anchored = 0x309;
                constexpr std::uint64_t cancollide = 0x30b;
            };

            namespace instancevalue {
                constexpr std::uint64_t value = 0xC8;
            };
        };

        namespace player {
            constexpr std::uint64_t localplayer = 0x1189;
            constexpr std::uint64_t character = 0x2a0;
            constexpr std::uint64_t userid = 0x1E8;
            constexpr std::uint64_t displayname = 0x100;
            constexpr std::uint64_t team = 0x1d8;
            constexpr std::uint64_t teamcolor = 0x274;
            constexpr std::uint64_t cameramaxzoom = 0x238;
            constexpr std::uint64_t cameraminzoom = 0x23C;

            namespace characterinstance {
                namespace humanoid {
                    constexpr std::uint64_t rigtype = 0x1B8;
                    constexpr std::uint64_t health = 0x18c;
                    constexpr std::uint64_t maxhealth = 0x1ac;
                    constexpr std::uint64_t walkspeed = 0x1C0;
                    constexpr std::uint64_t walkspeedcheck = 0x398;
                    constexpr std::uint64_t jumppower = 0x1A4;
                    constexpr std::uint64_t hipheight = 0x194;
                };
            };
        };
    };
};