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

        inline Int32 luaEntityType(lua_State * _state)
        {
            brick::TypedEntity * e = convertToTypeAndCheck<brick::TypedEntity>(_state, 1);
            lua_pushlightuserdata(_state, e->entityType());
            return 1;
        }

        //@TODO: Does this still need fixing?
        inline Int32 luaEntityCast(lua_State * _state)
        {
            luanatic::detail::LuanaticState * state = luanatic::detail::luanaticState(_state);
            STICK_ASSERT(state);
            if (lua_istable(_state, 1) && lua_isuserdata(_state, 2))
            {
                lua_getfield(_state, 1, "__entityType"); // 1 2 et
                if (lua_isuserdata(_state, -1))
                {
                    brick::Entity * e = convertToTypeAndCheck<brick::Entity>(_state, 2);
                    STICK_ASSERT(e);
                    STICK_ASSERT(e->isValid());
                    auto maybe = e->maybe<brick::detail::EntityTypeHolder>();
                    if (maybe && lua_touserdata(_state, -1) == *maybe)
                    {
                        lua_pushvalue(_state, 1); // 1 2 et 1
                        lua_setmetatable(_state, -3); // 1 2 et
                        lua_pop(_state, 1); // 1 2

                        luanatic::detail::UserData * ud = static_cast<luanatic::detail::UserData *>(lua_touserdata(_state, 2));
                        lua_getfield(_state, 1, "__typeID");
                        ud->m_typeID = reinterpret_cast<stick::TypeID>(lua_touserdata(_state, -1));
                        lua_pop(_state, 1);
                        return 1;
                    }
                }
            }

            lua_pushnil(_state); // 1 2 nil
            return 1;
        }

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
                luanatic::push<paper::Document>(_state, state->m_allocator->create<paper::Document>(createDocument()), true);
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
                if (str)
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

        inline Int32 luaParseSVG(lua_State * _state)
        {
            Document * doc = convertToTypeAndCheck<Document>(_state, 1);
            if (lua_isstring(_state, 2))
            {
                svg::SVGImportResult res;
                if(lua_isnumber(_state, 3))
                    res = doc->parseSVG(lua_tostring(_state, 2), lua_tonumber(_state, 3));
                else
                    res = doc->parseSVG(lua_tostring(_state, 2));
                if(res)
                {
                    lua_newtable(_state);
                    lua_pushnumber(_state, res.width());
                    lua_setfield(_state, -2, "width");
                    lua_pushnumber(_state, res.height());
                    lua_setfield(_state, -2, "height");
                    pushValueType<Group>(_state, res.group());
                    lua_setfield(_state, -2, "group");
                }
                else
                {
                    lua_newtable(_state);
                    pushValueType<Error>(_state, res.error());
                    lua_setfield(_state, -2, "error");
                }
            }
            else
            {
                luaL_argerror(_state, 1, "String expected.");
            }
            return 1;
        }

        inline Int32 luaLoadSVG(lua_State * _state)
        {
            Document * doc = convertToTypeAndCheck<Document>(_state, 1);
            if (lua_isstring(_state, 2))
            {
                svg::SVGImportResult res;
                if(lua_isnumber(_state, 3))
                    res = doc->loadSVG(URI(lua_tostring(_state, 2)), lua_tonumber(_state, 3));
                else
                    res = doc->loadSVG(URI(lua_tostring(_state, 2)));
                if(res)
                {
                    lua_newtable(_state);
                    lua_pushnumber(_state, res.width());
                    lua_setfield(_state, -2, "width");
                    lua_pushnumber(_state, res.height());
                    lua_setfield(_state, -2, "height");
                    pushValueType<Group>(_state, res.group());
                    lua_setfield(_state, -2, "group");
                }
                else
                {
                    lua_newtable(_state);
                    pushValueType<Error>(_state, res.error());
                    lua_setfield(_state, -2, "error");
                }
            }
            else
            {
                luaL_argerror(_state, 1, "String expected.");
            }
            return 1;
        }

        inline Int32 luaClosestCurveLocation(lua_State * _state)
        {
            Path * p = convertToTypeAndCheck<Path>(_state, 1);
            Float dist;
            auto cl = p->closestCurveLocation(luanatic::detail::Converter<const Vec2f &>::convert(_state, 2), dist);
            luanatic::pushValueType<CurveLocation>(_state, cl);
            lua_pushnumber(_state, dist);
            return 2;
        }

        inline Int32 luaIntersections(lua_State * _state)
        {
            Path * a = convertToTypeAndCheck<Path>(_state, 1);
            Path * b = convertToTypeAndCheck<Path>(_state, 2);
            auto inter = a->intersections(*b);
            lua_createtable(_state, inter.count(), 0);
            for (stick::Int32 i = 0; i < inter.count(); ++i)
            {
                lua_pushinteger(_state, i + 1);
                lua_newtable(_state);
                luanatic::pushValueType<CurveLocation>(_state, inter[i].location);
                lua_setfield(_state, -2, "location");
                luanatic::pushValueType<Vec2f>(_state, inter[i].position);
                lua_setfield(_state, -2, "position");
                lua_settable(_state, -3);
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

        LuaValue paintTypeTable = namespaceTable.findOrCreateTable("PaintType");
        paintTypeTable["None"].set(PaintType::None);
        paintTypeTable["Color"].set(PaintType::Color);
        paintTypeTable["LinearGradient"].set(PaintType::LinearGradient);
        paintTypeTable["CircularGradient"].set(PaintType::CircularGradient);

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
        //addConstructor("new").
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
        addMemberFunction("isCollinear", LUANATIC_FUNCTION(&Curve::isCollinear)).
        addMemberFunction("length", LUANATIC_FUNCTION(&Curve::length)).
        addMemberFunction("area", LUANATIC_FUNCTION(&Curve::area)).
        addMemberFunction("divideAt", LUANATIC_FUNCTION(&Curve::divideAt)).
        addMemberFunction("divideAtParameter", LUANATIC_FUNCTION(&Curve::divideAtParameter)).
        addMemberFunction("bounds", LUANATIC_FUNCTION_OVERLOAD(const Rect & (Curve::*)()const, &Curve::bounds)).
        addMemberFunction("boundsWithPadding", LUANATIC_FUNCTION_OVERLOAD(Rect(Curve::*)(Float)const, &Curve::bounds));

        namespaceTable.registerClass(curveCW);


        ClassWrapper<brick::Entity> entityCW("Entity");
        entityCW.
        addMemberFunction("id", LUANATIC_FUNCTION(&brick::Entity::id)).
        addMemberFunction("version", LUANATIC_FUNCTION(&brick::Entity::version));

        ClassWrapper<brick::TypedEntity> typedEntityCW("TypedEntity");
        typedEntityCW.
        addBase<brick::Entity>().
        addMemberFunction("entityType", detail::luaEntityType);

        ClassWrapper<brick::SharedTypedEntity> sharedTypedEntityCW("SharedTypedEntity");
        sharedTypedEntityCW.
        addBase<brick::Entity>().
        addMemberFunction("entityType", detail::luaEntityType);

        namespaceTable.registerClass(entityCW);
        namespaceTable.registerClass(typedEntityCW);
        namespaceTable.registerClass(sharedTypedEntityCW);
        namespaceTable.registerFunction("entityCast", detail::luaEntityCast);

        ClassWrapper<Paint> paintCW("Paint");
        paintCW.
        addBase<brick::SharedTypedEntity>().
        addMemberFunction("clone", LUANATIC_FUNCTION(&Paint::clone)).
        addMemberFunction("paintType", LUANATIC_FUNCTION(&Paint::paintType)).
        addMemberFunction("remove", LUANATIC_FUNCTION(&Paint::remove));

        namespaceTable.registerClass(paintCW);

        ClassWrapper<NoPaint> noPaintCW("NoPaint");
        noPaintCW.
        addBase<Paint>().
        addMemberFunction("clone", LUANATIC_FUNCTION(&NoPaint::clone));

        namespaceTable.registerClass(noPaintCW);
        namespaceTable["NoPaint"]["__entityType"].set(stick::TypeInfoT<NoPaint>::typeID());

        ClassWrapper<ColorPaint> colorPaintCW("ColorPaint");
        colorPaintCW.
        addBase<Paint>().
        addMemberFunction("clone", LUANATIC_FUNCTION(&ColorPaint::clone)).
        addMemberFunction("setColor", LUANATIC_FUNCTION(&ColorPaint::setColor)).
        addMemberFunction("color", LUANATIC_FUNCTION(&ColorPaint::color));

        namespaceTable.registerClass(colorPaintCW);
        namespaceTable["ColorPaint"]["__entityType"].set(stick::TypeInfoT<ColorPaint>::typeID());

        ClassWrapper<Item> itemCW("Item");
        itemCW.
        addBase<brick::TypedEntity>().
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
        addMemberFunction("scaleAroundTransform", LUANATIC_FUNCTION_OVERLOAD(void(Item::*)(const Vec2f &, const Vec2f &), &Item::scaleTransform)).
        addMemberFunction("rotateTransform", LUANATIC_FUNCTION_OVERLOAD(void(Item::*)(Float), &Item::rotateTransform)).
        addMemberFunction("rotateAroundTransform", LUANATIC_FUNCTION_OVERLOAD(void(Item::*)(Float, const Vec2f &), &Item::rotateTransform)).
        addMemberFunction("transformItem", LUANATIC_FUNCTION_OVERLOAD(void(Item::*)(const Mat3f &, bool), &Item::transform)).
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
        addMemberFunction("setStrokeFromColor", LUANATIC_FUNCTION_OVERLOAD(Paint(Item::*)(const ColorRGBA &), &Item::setStroke)).
        addMemberFunction("setStrokeFromString", LUANATIC_FUNCTION_OVERLOAD(Paint(Item::*)(const stick::String &), &Item::setStroke)).
        addMemberFunction("setStroke", LUANATIC_FUNCTION_OVERLOAD(Paint(Item::*)(const ColorRGBA &), &Item::setStroke)).
        addMemberFunction("setStroke", LUANATIC_FUNCTION_OVERLOAD(Paint(Item::*)(const stick::String &), &Item::setStroke)).
        addMemberFunction("removeStroke", LUANATIC_FUNCTION(&Item::removeStroke)).
        addMemberFunction("setFillFromColor", LUANATIC_FUNCTION_OVERLOAD(Paint(Item::*)(const ColorRGBA &), &Item::setFill)).
        addMemberFunction("setFillFromString", LUANATIC_FUNCTION_OVERLOAD(Paint(Item::*)(const stick::String &), &Item::setFill)).
        addMemberFunction("setFill", LUANATIC_FUNCTION_OVERLOAD(Paint(Item::*)(const ColorRGBA &), &Item::setFill)).
        addMemberFunction("setFill", LUANATIC_FUNCTION_OVERLOAD(Paint(Item::*)(const stick::String &), &Item::setFill)).
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
        addMemberFunction("itemType", LUANATIC_FUNCTION(&Item::itemType)).
        addMemberFunction("children", LUANATIC_FUNCTION(&Item::children, ReturnRefIterator<ph::Result>));

        namespaceTable.registerClass(itemCW);

        ClassWrapper<Group> groupCW("Group");
        groupCW.
        addBase<Item>().
        addMemberFunction("setClipped", LUANATIC_FUNCTION(&Group::setClipped)).
        addMemberFunction("isClipped", LUANATIC_FUNCTION(&Group::isClipped)).
        addMemberFunction("clone", LUANATIC_FUNCTION(&Group::clone));

        namespaceTable.registerClass(groupCW);
        namespaceTable["Group"]["__entityType"].set(stick::TypeInfoT<Group>::typeID());

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
        addMemberFunction("segments", LUANATIC_FUNCTION_OVERLOAD(Path::SegmentView (Path::*)(), &Path::segments, ReturnRefIterator<ph::Result>)).
        addMemberFunction("curves", LUANATIC_FUNCTION_OVERLOAD(Path::CurveView (Path::*)(), &Path::curves, ReturnRefIterator<ph::Result>)).
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
        addMemberFunction("closestCurveLocation", &detail::luaClosestCurveLocation).
        addMemberFunction("curveLocationAt", LUANATIC_FUNCTION(&Path::curveLocationAt)).
        addMemberFunction("length", LUANATIC_FUNCTION(&Path::length)).
        addMemberFunction("area", LUANATIC_FUNCTION(&Path::area)).
        addMemberFunction("extrema", LUANATIC_FUNCTION_OVERLOAD(stick::DynamicArray<CurveLocation> (Path::*)()const, &Path::extrema)).
        addMemberFunction("isClosed", LUANATIC_FUNCTION(&Path::isClosed)).
        addMemberFunction("isClockwise", LUANATIC_FUNCTION(&Path::isClockwise)).
        addMemberFunction("contains", LUANATIC_FUNCTION(&Path::contains)).
        addMemberFunction("segment", LUANATIC_FUNCTION_OVERLOAD(Segment & (Path::*)(stick::Size), &Path::segment)).
        addMemberFunction("curve", LUANATIC_FUNCTION_OVERLOAD(Curve & (Path::*)(stick::Size), &Path::curve)).
        addMemberFunction("segmentCount", LUANATIC_FUNCTION(&Path::segmentCount)).
        addMemberFunction("curveCount", LUANATIC_FUNCTION(&Path::curveCount)).
        addMemberFunction("clone", LUANATIC_FUNCTION(&Path::clone)).
        addMemberFunction("intersections", detail::luaIntersections).
        addMemberFunction("slice", LUANATIC_FUNCTION_OVERLOAD(Path(Path::*)(const CurveLocation &, const CurveLocation &)const, &Path::slice)).
        addMemberFunction("slice", LUANATIC_FUNCTION_OVERLOAD(Path(Path::*)(Float, Float)const, &Path::slice)).
        addMemberFunction("sliceAtLocations", LUANATIC_FUNCTION_OVERLOAD(Path(Path::*)(const CurveLocation &, const CurveLocation &)const, &Path::slice)).
        addMemberFunction("sliceAtOffsets", LUANATIC_FUNCTION_OVERLOAD(Path(Path::*)(Float, Float)const, &Path::slice));

        namespaceTable.registerClass(pathCW);
        namespaceTable["Path"]["__entityType"].set(stick::TypeInfoT<Path>::typeID());

        ClassWrapper<Document> docCW("Document");
        docCW.
        addBase<Item>().
        addConstructor<>().
        addConstructor("new").
        addMemberFunction("createGroup", LUANATIC_FUNCTION(&Document::createGroup), "").
        addMemberFunction("createPath", LUANATIC_FUNCTION(&Document::createPath), "").
        addMemberFunction("createEllipse", LUANATIC_FUNCTION(&Document::createEllipse), "").
        addMemberFunction("createCircle", LUANATIC_FUNCTION(&Document::createCircle), "").
        addMemberFunction("createRectangle", LUANATIC_FUNCTION(&Document::createRectangle), "").
        addMemberFunction("setSize", LUANATIC_FUNCTION(&Document::setSize)).
        addMemberFunction("width", LUANATIC_FUNCTION(&Document::width)).
        addMemberFunction("height", LUANATIC_FUNCTION(&Document::height)).
        addMemberFunction("size", LUANATIC_FUNCTION(&Document::size)).
        addMemberFunction("saveSVG", detail::luaSaveSVG).
        addMemberFunction("parseSVG", detail::luaParseSVG).
        addMemberFunction("loadSVG", detail::luaLoadSVG);

        namespaceTable.registerClass(docCW);
        namespaceTable["Document"]["__entityType"].set(stick::TypeInfoT<Document>::typeID());
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
        addConstructor<const Document &>().
        addConstructor<const Document &>("new");

        namespaceTable.registerClass(glRendererCW);
    }
}

#endif //PAPERLUA_PAPERLUA_HPP
