#include <Stick/Test.hpp>
#include <PaperLua/PaperLua.hpp>

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

            EXPECT(!err);
        }
        lua_close(state);
    }
};

int main(int _argc, const char * _args[])
{
    return runTests(spec, _argc, _args);
}
