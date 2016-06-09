#ifndef PAPERLUA_PAPERLUA_HPP
#define PAPERLUA_PAPERLUA_HPP

#include <Luanatic/Luanatic.hpp>
#include <Paper/Document.hpp>
#include <Paper/Constants.hpp>
#include <Paper/CurveLocation.hpp>
#include <Paper/OpenGL/GLRenderer.hpp>
#include <Stick/Path.hpp>

namespace paperLua
{
    STICK_API inline void registerPaper(lua_State * _state, const stick::String & _namespace = "");

    namespace detail
    {
        using namespace stick;
        using namespace luanatic;
        using namespace paper;

        inline Int32 luaCreateDocument(lua_State * _state)
        {
            luanatic::detail::LuanaticState * state = luanatic::detail::luanaticState(_state);
            STICK_ASSERT(state);
            if (lua_isstring(_state, 1))
            {
                luanatic::push<paper::Document>(_state, state->m_allocator->create<paper::Document>(createDocument(defaultHub(), lua_tostring(_state, 1))), true);
            }
            else
            {
                luanatic::push<paper::Document>(_state, state->m_allocator->create<paper::Document>(paper::createDocument()), true);
            }
            return 1;
        }

        inline Int32 luaSaveSVG(lua_State * _state)
        {
            Document * doc = convertToTypeAndCheck<Document>(_state, 1);
            if (lua_isstring(_state, 2))
            {
                pushValueType<Error>(_state, doc->saveSVG(URI(lua_tostring(_state, 2))));
            }
            else
            {
                String * str = convertToTypeAndCheck<String>(_state, 2);
                if(str)
                {
                    pushValueType<Error>(_state, doc->saveSVG(URI(*str)));
                }
                else
                {
                    luaL_argerror(_state, 1, "String expected.");
                }
            }
            return 1;
        }
    }

    inline void registerPaper(lua_State * _state, const stick::String & _namespace)
    {
        using namespace luanatic;
        using namespace paper;
        using namespace stick;

        LuaValue g = globalsTable(_state);
        LuaValue namespaceTable = g;
        if (!_namespace.isEmpty())
        {
            auto tokens = path::segments(_namespace, '.');
            for (const String & token : tokens)
            {
                LuaValue table = namespaceTable.findOrCreateTable(token);
                namespaceTable = table;
            }
        }

        //register constants
        LuaValue strokeJoinTable = namespaceTable.findOrCreateTable("StrokeJoin");
        strokeJoinTable["Miter"].set(StrokeJoin::Miter);
        strokeJoinTable["Round"].set(StrokeJoin::Round);
        strokeJoinTable["Bevel"].set(StrokeJoin::Bevel);

        LuaValue strokeCapTable = namespaceTable.findOrCreateTable("StrokeCap");
        strokeCapTable["Round"].set(StrokeCap::Round);
        strokeCapTable["Square"].set(StrokeCap::Square);
        strokeCapTable["Butt"].set(StrokeCap::Butt);

        LuaValue windingRuleTable = namespaceTable.findOrCreateTable("WindingRule");
        windingRuleTable["EvenOdd"].set(WindingRule::EvenOdd);
        windingRuleTable["NonZero"].set(WindingRule::NonZero);

        LuaValue entityTypeTable = namespaceTable.findOrCreateTable("EntityType");
        entityTypeTable["Document"].set(EntityType::Document);
        entityTypeTable["Path"].set(EntityType::Path);
        entityTypeTable["Group"].set(EntityType::Group);

        LuaValue smoothingTypeTable = namespaceTable.findOrCreateTable("Smoothing");
        smoothingTypeTable["Continuous"].set(Smoothing::Continuous);
        smoothingTypeTable["Asymmetric"].set(Smoothing::Asymmetric);
        smoothingTypeTable["CatmullRom"].set(Smoothing::CatmullRom);
        smoothingTypeTable["Geometric"].set(Smoothing::Geometric);

        ClassWrapper<Segment> segmentCW("Segment");
        segmentCW.
        addMemberFunction("setPosition", LUANATIC_FUNCTION(&Segment::setPosition)).
        addMemberFunction("setHandleIn", LUANATIC_FUNCTION(&Segment::setHandleIn)).
        addMemberFunction("setHandleOut", LUANATIC_FUNCTION(&Segment::setHandleOut)).
        addMemberFunction("position", LUANATIC_FUNCTION(&Segment::position)).
        addMemberFunction("handleIn", LUANATIC_FUNCTION(&Segment::handleIn)).
        addMemberFunction("handleOut", LUANATIC_FUNCTION(&Segment::handleOut)).
        addMemberFunction("handleInAbsolute", LUANATIC_FUNCTION(&Segment::handleInAbsolute)).
        addMemberFunction("handleOutAbsolute", LUANATIC_FUNCTION(&Segment::handleOutAbsolute)).
        addMemberFunction("isLinear", LUANATIC_FUNCTION(&Segment::isLinear)).
        addMemberFunction("remove", LUANATIC_FUNCTION(&Segment::remove));

        namespaceTable.registerClass(segmentCW);

        ClassWrapper<CurveLocation> curveLocationCW("CurveLocation");
        curveLocationCW.
        addConstructor("new").
        addMemberFunction("position", LUANATIC_FUNCTION(&CurveLocation::position)).
        addMemberFunction("normal", LUANATIC_FUNCTION(&CurveLocation::normal)).
        addMemberFunction("tangent", LUANATIC_FUNCTION(&CurveLocation::tangent)).
        addMemberFunction("curvature", LUANATIC_FUNCTION(&CurveLocation::curvature)).
        addMemberFunction("angle", LUANATIC_FUNCTION(&CurveLocation::angle)).
        addMemberFunction("parameter", LUANATIC_FUNCTION(&CurveLocation::parameter)).
        addMemberFunction("offset", LUANATIC_FUNCTION(&CurveLocation::offset)).
        addMemberFunction("isValid", LUANATIC_FUNCTION(&CurveLocation::isValid)).
        addMemberFunction("curve", LUANATIC_FUNCTION(&CurveLocation::curve));

        namespaceTable.registerClass(curveLocationCW);

        ClassWrapper<Curve> curveCW("Curve");
        curveCW.
        addMemberFunction("path", LUANATIC_FUNCTION(&Curve::path)).
        addMemberFunction("setPositionOne", LUANATIC_FUNCTION(&Curve::setPositionOne)).
        addMemberFunction("setHandleOne", LUANATIC_FUNCTION(&Curve::setHandleOne)).
        addMemberFunction("setPositionTwo", LUANATIC_FUNCTION(&Curve::setPositionTwo)).
        addMemberFunction("setHandleTwo", LUANATIC_FUNCTION(&Curve::setHandleTwo)).
        addMemberFunction("positionOne", LUANATIC_FUNCTION(&Curve::positionOne)).
        addMemberFunction("positionTwo", LUANATIC_FUNCTION(&Curve::positionTwo)).
        addMemberFunction("handleOne", LUANATIC_FUNCTION(&Curve::handleOne)).
        addMemberFunction("handleOneAbsolute", LUANATIC_FUNCTION(&Curve::handleOneAbsolute)).
        addMemberFunction("handleTwo", LUANATIC_FUNCTION(&Curve::handleTwo)).
        addMemberFunction("handleTwoAbsolute", LUANATIC_FUNCTION(&Curve::handleTwoAbsolute)).
        addMemberFunction("positionAt", LUANATIC_FUNCTION(&Curve::positionAt)).
        addMemberFunction("normalAt", LUANATIC_FUNCTION(&Curve::normalAt)).
        addMemberFunction("tangentAt", LUANATIC_FUNCTION(&Curve::tangentAt)).
        addMemberFunction("curvatureAt", LUANATIC_FUNCTION(&Curve::curvatureAt)).
        addMemberFunction("angleAt", LUANATIC_FUNCTION(&Curve::angleAt)).
        addMemberFunction("positionAtParameter", LUANATIC_FUNCTION(&Curve::positionAtParameter)).
        addMemberFunction("normalAtParameter", LUANATIC_FUNCTION(&Curve::normalAtParameter)).
        addMemberFunction("tangentAtParameter", LUANATIC_FUNCTION(&Curve::tangentAtParameter)).
        addMemberFunction("curvatureAtParameter", LUANATIC_FUNCTION(&Curve::curvatureAtParameter)).
        addMemberFunction("angleAtParameter", LUANATIC_FUNCTION(&Curve::angleAtParameter)).
        addMemberFunction("parameterAtOffset", LUANATIC_FUNCTION(&Curve::parameterAtOffset)).
        addMemberFunction("closestParameter", LUANATIC_FUNCTION_OVERLOAD(Float(Curve::*)(const Vec2f &)const, &Curve::closestParameter)).
        addMemberFunction("lengthBetween", LUANATIC_FUNCTION(&Curve::lengthBetween)).
        addMemberFunction("pathOffset", LUANATIC_FUNCTION(&Curve::pathOffset)).
        addMemberFunction("closestCurveLocation", LUANATIC_FUNCTION(&Curve::closestCurveLocation)).
        addMemberFunction("curveLocationAt", LUANATIC_FUNCTION(&Curve::curveLocationAt)).
        addMemberFunction("curveLocationAtParameter", LUANATIC_FUNCTION(&Curve::curveLocationAtParameter)).
        addMemberFunction("isLinear", LUANATIC_FUNCTION(&Curve::isLinear)).
        addMemberFunction("isStraight", LUANATIC_FUNCTION(&Curve::isStraight)).
        addMemberFunction("isArc", LUANATIC_FUNCTION(&Curve::isArc)).
        addMemberFunction("isOrthogonal", LUANATIC_FUNCTION(&Curve::isOrthogonal)).
        addMemberFunction("isColinear", LUANATIC_FUNCTION(&Curve::isColinear)).
        addMemberFunction("length", LUANATIC_FUNCTION(&Curve::length)).
        addMemberFunction("area", LUANATIC_FUNCTION(&Curve::area)).
        addMemberFunction("bounds", LUANATIC_FUNCTION_OVERLOAD(const Rect & (Curve::*)()const, &Curve::bounds)).
        addMemberFunction("boundsWithPadding", LUANATIC_FUNCTION_OVERLOAD(Rect(Curve::*)(Float)const, &Curve::bounds));

        namespaceTable.registerClass(curveCW);


        ClassWrapper<brick::Entity> entityCW("Entity");
        namespaceTable.registerClass(entityCW);

        ClassWrapper<Item> itemCW("Item");
        itemCW.
        addBase<brick::Entity>().
        addMemberFunction("addChild", LUANATIC_FUNCTION(&Item::addChild)).
        addMemberFunction("insertAbove", LUANATIC_FUNCTION(&Item::insertAbove)).
        addMemberFunction("insertBelow", LUANATIC_FUNCTION(&Item::insertBelow)).
        addMemberFunction("sendToFront", LUANATIC_FUNCTION(&Item::sendToFront)).
        addMemberFunction("sendToBack", LUANATIC_FUNCTION(&Item::sendToBack)).
        addMemberFunction("reverseChildren", LUANATIC_FUNCTION(&Item::reverseChildren)).
        addMemberFunction("remove", LUANATIC_FUNCTION(&Item::remove)).
        addMemberFunction("removeChildren", LUANATIC_FUNCTION(&Item::removeChildren)).
        addMemberFunction("name", LUANATIC_FUNCTION(&Item::name)).
        addMemberFunction("parent", LUANATIC_FUNCTION(&Item::parent)).
        addMemberFunction("setPosition", LUANATIC_FUNCTION(&Item::setPosition)).
        addMemberFunction("setPivot", LUANATIC_FUNCTION(&Item::setPivot)).
        addMemberFunction("setVisible", LUANATIC_FUNCTION(&Item::setVisible)).
        addMemberFunction("setName", LUANATIC_FUNCTION(&Item::setName)).
        addMemberFunction("setTransform", LUANATIC_FUNCTION(&Item::setTransform)).
        addMemberFunction("translateTransform", LUANATIC_FUNCTION_OVERLOAD(void(Item::*)(const Vec2f &), &Item::translateTransform)).
        addMemberFunction("scaleTransform", LUANATIC_FUNCTION_OVERLOAD(void(Item::*)(const Vec2f &), &Item::scaleTransform)).
        addMemberFunction("scaleAroundTransform", LUANATIC_FUNCTION_OVERLOAD(void(Item::*)(const Vec2f &, const Vec2f&), &Item::scaleTransform)).
        addMemberFunction("rotateTransform", LUANATIC_FUNCTION_OVERLOAD(void(Item::*)(Float), &Item::rotateTransform)).
        addMemberFunction("rotateAroundTransform", LUANATIC_FUNCTION_OVERLOAD(void(Item::*)(Float, const Vec2f &), &Item::rotateTransform)).
        addMemberFunction("transformItem", LUANATIC_FUNCTION_OVERLOAD(void(Item::*)(const Mat3f &), &Item::transform)).
        addMemberFunction("translate", LUANATIC_FUNCTION_OVERLOAD(void(Item::*)(const Vec2f &), &Item::translate)).
        addMemberFunction("scale", LUANATIC_FUNCTION_OVERLOAD(void(Item::*)(const Vec2f &), &Item::scale)).
        addMemberFunction("rotate", LUANATIC_FUNCTION_OVERLOAD(void(Item::*)(Float), &Item::rotate)).
        addMemberFunction("rotateAround", LUANATIC_FUNCTION_OVERLOAD(void(Item::*)(Float, const Vec2f &), &Item::rotate)).
        addMemberFunction("applyTransform", LUANATIC_FUNCTION(&Item::applyTransform)).
        addMemberFunction("transform", LUANATIC_FUNCTION_OVERLOAD(const Mat3f & (Item::*)()const, &Item::transform)).
        addMemberFunction("rotation", LUANATIC_FUNCTION(&Item::rotation)).
        addMemberFunction("translation", LUANATIC_FUNCTION(&Item::translation)).
        addMemberFunction("scaling", LUANATIC_FUNCTION(&Item::scaling)).
        addMemberFunction("absoluteRotation", LUANATIC_FUNCTION(&Item::absoluteRotation)).
        addMemberFunction("absoluteTranslation", LUANATIC_FUNCTION(&Item::absoluteTranslation)).
        addMemberFunction("absoluteScaling", LUANATIC_FUNCTION(&Item::absoluteScaling)).
        addMemberFunction("bounds", LUANATIC_FUNCTION(&Item::bounds)).
        addMemberFunction("localBounds", LUANATIC_FUNCTION(&Item::localBounds)).
        addMemberFunction("handleBounds", LUANATIC_FUNCTION(&Item::handleBounds)).
        addMemberFunction("strokeBounds", LUANATIC_FUNCTION(&Item::strokeBounds)).
        addMemberFunction("position", LUANATIC_FUNCTION(&Item::position)).
        addMemberFunction("pivot", LUANATIC_FUNCTION(&Item::pivot)).
        addMemberFunction("isVisible", LUANATIC_FUNCTION(&Item::isVisible)).
        addMemberFunction("setStrokeJoin", LUANATIC_FUNCTION(&Item::setStrokeJoin)).
        addMemberFunction("setStrokeCap", LUANATIC_FUNCTION(&Item::setStrokeCap)).
        addMemberFunction("setMiterLimit", LUANATIC_FUNCTION(&Item::setMiterLimit)).
        addMemberFunction("setStrokeWidth", LUANATIC_FUNCTION(&Item::setStrokeWidth)).
        addMemberFunction("setDashArray", LUANATIC_FUNCTION(&Item::setDashArray)).
        addMemberFunction("setDashOffset", LUANATIC_FUNCTION(&Item::setDashOffset)).
        addMemberFunction("setStrokeScaling", LUANATIC_FUNCTION(&Item::setStrokeScaling)).
        addMemberFunction("setStroke", LUANATIC_FUNCTION(&Item::setStroke)).
        addMemberFunction("removeStroke", LUANATIC_FUNCTION(&Item::removeStroke)).
        addMemberFunction("setFill", LUANATIC_FUNCTION(&Item::setFill)).
        addMemberFunction("removeFill", LUANATIC_FUNCTION(&Item::removeFill)).
        addMemberFunction("setWindingRule", LUANATIC_FUNCTION(&Item::setWindingRule)).
        addMemberFunction("strokeJoin", LUANATIC_FUNCTION(&Item::strokeJoin)).
        addMemberFunction("strokeCap", LUANATIC_FUNCTION(&Item::strokeCap)).
        addMemberFunction("miterLimit", LUANATIC_FUNCTION(&Item::miterLimit)).
        addMemberFunction("strokeWidth", LUANATIC_FUNCTION(&Item::strokeWidth)).
        addMemberFunction("dashArray", LUANATIC_FUNCTION(&Item::dashArray)).
        addMemberFunction("dashOffset", LUANATIC_FUNCTION(&Item::dashOffset)).
        addMemberFunction("windingRule", LUANATIC_FUNCTION(&Item::windingRule)).
        addMemberFunction("isScalingStroke", LUANATIC_FUNCTION(&Item::isScalingStroke)).
        addMemberFunction("fill", LUANATIC_FUNCTION(&Item::fill)).
        addMemberFunction("stroke", LUANATIC_FUNCTION(&Item::stroke)).
        addMemberFunction("hasStroke", LUANATIC_FUNCTION(&Item::hasStroke)).
        addMemberFunction("hasFill", LUANATIC_FUNCTION(&Item::hasFill)).
        addMemberFunction("clone", LUANATIC_FUNCTION(&Item::clone)).
        addMemberFunction("document", LUANATIC_FUNCTION(&Item::document)).
        addMemberFunction("itemType", LUANATIC_FUNCTION(&Item::itemType));

        namespaceTable.registerClass(itemCW);

        ClassWrapper<Group> groupCW("Group");
        groupCW.
        addBase<Item>().
        addMemberFunction("setClipped", LUANATIC_FUNCTION(&Group::setClipped)).
        addMemberFunction("isClipped", LUANATIC_FUNCTION(&Group::isClipped)).
        addMemberFunction("clone", LUANATIC_FUNCTION(&Group::clone));

        namespaceTable.registerClass(groupCW);

        ClassWrapper<Path> pathCW("Path");
        pathCW.
        addBase<Item>().
        addMemberFunction("addPoint", LUANATIC_FUNCTION(&Path::addPoint)).
        addMemberFunction("cubicCurveTo", LUANATIC_FUNCTION(&Path::cubicCurveTo)).
        addMemberFunction("quadraticCurveTo", LUANATIC_FUNCTION(&Path::quadraticCurveTo)).
        addMemberFunction("curveTo", LUANATIC_FUNCTION(&Path::curveTo)).
        addMemberFunction("arcThrough", LUANATIC_FUNCTION_OVERLOAD(Error(Path::*)(const Vec2f &, const Vec2f &), &Path::arcTo)).
        addMemberFunction("arcTo", LUANATIC_FUNCTION_OVERLOAD(Error(Path::*)(const Vec2f &, bool), &Path::arcTo)).
        addMemberFunction("cubicCurveBy", LUANATIC_FUNCTION(&Path::cubicCurveBy)).
        addMemberFunction("quadraticCurveBy", LUANATIC_FUNCTION(&Path::quadraticCurveBy)).
        addMemberFunction("curveBy", LUANATIC_FUNCTION(&Path::curveBy)).
        addMemberFunction("closePath", LUANATIC_FUNCTION(&Path::closePath)).
        addMemberFunction("smooth", LUANATIC_FUNCTION_OVERLOAD(void(Path::*)(Smoothing), &Path::smooth)).
        addMemberFunction("smoothFromTo", LUANATIC_FUNCTION_OVERLOAD(void(Path::*)(Int64, Int64, Smoothing), &Path::smooth)).
        addMemberFunction("simplify", LUANATIC_FUNCTION(&Path::simplify)).
        addMemberFunction("addSegment", LUANATIC_FUNCTION(&Path::addSegment)).
        addMemberFunction("removeSegment", LUANATIC_FUNCTION(&Path::removeSegment)).
        addMemberFunction("removeSegmentsFrom", LUANATIC_FUNCTION_OVERLOAD(void(Path::*)(Size), &Path::removeSegments)).
        addMemberFunction("removeSegmentsFromTo", LUANATIC_FUNCTION_OVERLOAD(void(Path::*)(Size, Size), &Path::removeSegments)).
        addMemberFunction("removeSegments", LUANATIC_FUNCTION_OVERLOAD(void(Path::*)(), &Path::removeSegments)).
        addMemberFunction("segments", LUANATIC_RETURN_REF_ITERATOR_OVERLOAD(SegmentArray & (Path::*)(), &Path::segments)).
        addMemberFunction("curves", LUANATIC_RETURN_REF_ITERATOR_OVERLOAD(CurveArray & (Path::*)(), &Path::curves)).
        addMemberFunction("positionAt", LUANATIC_FUNCTION(&Path::positionAt)).
        addMemberFunction("normalAt", LUANATIC_FUNCTION(&Path::normalAt)).
        addMemberFunction("tangentAt", LUANATIC_FUNCTION(&Path::tangentAt)).
        addMemberFunction("curvatureAt", LUANATIC_FUNCTION(&Path::curvatureAt)).
        addMemberFunction("angleAt", LUANATIC_FUNCTION(&Path::angleAt)).
        addMemberFunction("reverse", LUANATIC_FUNCTION(&Path::reverse)).
        addMemberFunction("setClockwise", LUANATIC_FUNCTION(&Path::setClockwise)).
        addMemberFunction("flatten", LUANATIC_FUNCTION(&Path::flatten)).
        addMemberFunction("flattenRegular", LUANATIC_FUNCTION(&Path::flattenRegular)).
        addMemberFunction("regularOffset", LUANATIC_FUNCTION(&Path::regularOffset)).
        addMemberFunction("closestCurveLocation", LUANATIC_FUNCTION_OVERLOAD(CurveLocation(Path::*)(const Vec2f &) const, &Path::closestCurveLocation)).
        addMemberFunction("curveLocationAt", LUANATIC_FUNCTION(&Path::curveLocationAt)).
        addMemberFunction("length", LUANATIC_FUNCTION(&Path::length)).
        addMemberFunction("area", LUANATIC_FUNCTION(&Path::area)).
        addMemberFunction("isClosed", LUANATIC_FUNCTION(&Path::isClosed)).
        addMemberFunction("isClockwise", LUANATIC_FUNCTION(&Path::isClockwise)).
        addMemberFunction("contains", LUANATIC_FUNCTION(&Path::contains)).
        addMemberFunction("clone", LUANATIC_FUNCTION(&Path::clone));

        namespaceTable.registerClass(pathCW);

        ClassWrapper<Document> docCW("Document");
        docCW.
        addBase<Item>().
        addConstructor("new").
        addMemberFunction("createGroup", LUANATIC_FUNCTION(&Document::createGroup)).
        addMemberFunction("createPath", LUANATIC_FUNCTION(&Document::createPath)).
        addMemberFunction("createEllipse", LUANATIC_FUNCTION(&Document::createEllipse)).
        addMemberFunction("createCircle", LUANATIC_FUNCTION(&Document::createCircle)).
        addMemberFunction("createRectangle", LUANATIC_FUNCTION(&Document::createRectangle)).
        addMemberFunction("setSize", LUANATIC_FUNCTION(&Document::setSize)).
        addMemberFunction("width", LUANATIC_FUNCTION(&Document::width)).
        addMemberFunction("height", LUANATIC_FUNCTION(&Document::height)).
        addMemberFunction("size", LUANATIC_FUNCTION(&Document::size)).
        addMemberFunction("saveSVG", detail::luaSaveSVG);

        namespaceTable.registerClass(docCW);
        namespaceTable.registerFunction("createDocument", detail::luaCreateDocument);

        ClassWrapper<RenderInterface> rendererCW("RenderInterface");
        rendererCW.
        addMemberFunction("setViewport", LUANATIC_FUNCTION(&RenderInterface::setViewport)).
        addMemberFunction("setTransform", LUANATIC_FUNCTION(&RenderInterface::setTransform)).
        addMemberFunction("draw", LUANATIC_FUNCTION(&RenderInterface::draw));

        namespaceTable.registerClass(rendererCW);

        //TODO only register the GL renderer on platforms that support it.
        //or add flags for which renderer to register.
        ClassWrapper<opengl::GLRenderer> glRendererCW("GLRenderer");
        glRendererCW.
        addBase<RenderInterface>().
        addConstructor<const Document &>("new");

        namespaceTable.registerClass(glRendererCW);

        printf("REGISTER PAPER END \n");
    }
}

#endif //PAPERLUA_PAPERLUA_HPP
