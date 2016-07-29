#include <Stick/Test.hpp>
#include <PaperLua/PaperLua.hpp>
#include <CrunchLua/CrunchLua.hpp>

using namespace stick;
using namespace paperLua;
using namespace luanatic;

const Suite spec[] =
{
    SUITE("Namespacing Tests")
    {
        lua_State * state = createLuaState();
        {
            openStandardLibraries(state);
            initialize(state);
            registerPaper(state, "paper");

            EXPECT(lua_gettop(state) == 0);

            //very basic test to see if the namespacing works
            String basicTest =
            "print(type(paper.Document))\n"
            "local doc = paper.createDocument(\"Doc\")\n"
            "assert(doc:name() == \"Doc\")\n"
            "local renderer = paper.GLRenderer.new(doc)\n"
            "renderer:setViewport(100, 100)\n";

            auto err = luanatic::execute(state, basicTest);
            if (err)
                printf("%s\n", err.message().cString());

            EXPECT(lua_gettop(state) == 0);

            EXPECT(!err);
        }
        lua_close(state);
    },
    SUITE("Path Tests")
    {
        lua_State * state = createLuaState();
        {
            openStandardLibraries(state);
            initialize(state);
            registerPaper(state);
            crunchLua::registerCrunch(state);

            EXPECT(lua_gettop(state) == 0);

            String basicTest =
                "local doc = createDocument(\"Doc\")\n"
                "assert(doc:name() == \"Doc\")\n"
                "local renderer = GLRenderer.new(doc)\n"
                "renderer:setViewport(100, 100)\n"
                "local p = doc:createCircle(Vec2.fromNumbers(100, 100), 10, \"Circle\")\n"
                "local val = p:normalAt(p:length() * 0.5)\n"
                "p:setFill(ColorRGBA.fromNumbers(1, 0, 0, 1))\n";

            auto err = luanatic::execute(state, basicTest);
            if (err)
                printf("%s\n", err.message().cString());

            EXPECT(lua_gettop(state) == 0);

            EXPECT(!err);
        }
        lua_close(state);
    }
};

int main(int _argc, const char * _args[])
{
    return runTests(spec, _argc, _args);
}
