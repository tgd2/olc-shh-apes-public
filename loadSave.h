#pragma once

#include <regex>
#include <fstream>

#include "types.h"

#include "json\tileson.hpp"

namespace LoadSave
{
	// Forward declarations ==================================================
	levelRecipe LoadLevelRecipe(std::string Filename);
	template <typename T> T GetProperty(tson::Object Obj, std::string PropName);
	std::vector<movementRecipe> ParseMovementsProperty(std::string PropValue);
	// =======================================================================

	constexpr unsigned FLIPPED_HORIZONTALLY_FLAG{ 0x80000000 };
	constexpr unsigned FLIPPED_VERTICALLY_FLAG{ 0x40000000 };
	constexpr unsigned FLIPPED_DIAGONALLY_FLAG{ 0x20000000 };
	constexpr unsigned ROTATED_HEXAGONAL_120_FLAG{ 0x10000000 };
	constexpr unsigned TILE_ID_MASK{ ~(FLIPPED_HORIZONTALLY_FLAG | FLIPPED_VERTICALLY_FLAG | FLIPPED_DIAGONALLY_FLAG | ROTATED_HEXAGONAL_120_FLAG) };

	levelRecipe LoadLevelRecipe(std::string Filename)
	{
		levelRecipe NewLevelRecipe{};
		NewLevelRecipe.Filename = Filename;

		tson::Tileson TilesonData;
		std::unique_ptr<tson::Map> LevelData = TilesonData.parse("./assets/levels/" + Filename);
		assert(LevelData->getStatus() == tson::ParseStatus::OK);

		NewLevelRecipe.Size = vec2{ (float)LevelData->getSize().x, (float)LevelData->getSize().y }; // Grid size is already in "tiles"

		tson::PropertyCollection MapProperties{ LevelData->getProperties() };
		if (MapProperties.hasProperty("DefaultPlatformClass"))
		{
			tson::Property* Prop = MapProperties.getProperty("DefaultPlatformClass");
			NewLevelRecipe.DefaultPlatformClass = std::any_cast<std::string>(Prop->getValue());
		}

		bool bHaveProcessedGameLayer{ false };
		int PlatformCounter{ 0 };

		// ============================================================================
		// TILE SETS
		// ============================================================================
		std::unordered_map<uint32_t, std::string> ObjectTiles{};
		for (int TileSetIndex = 0; TileSetIndex < LevelData->getTilesets().size(); ++TileSetIndex)
		{
			auto& TileSet{ LevelData->getTilesets().at(TileSetIndex)};
			int FirstGid{ TileSet.getFirstgid() };

			for (auto& Tile : TileSet.getTiles())
			{
				uint32_t TileGid{ Tile.getGid() & TILE_ID_MASK }; // ?
				std::string TileFilename{ Tile.getImage().string() };
				assert(TileFilename.substr(0, 2) == ".."); // ** Assumes TileFilename starts with '..'
				if (!TileFilename.empty()) TileFilename.erase(0, 1); // ** Remove initial '.'
				ObjectTiles.insert({ TileGid, TileFilename });

				vec2 ImageSize = vec2{ (float)Tile.getImageSize().x, (float)Tile.getImageSize().y };

				if (Tile.getObjectgroup().getObjects().size() > 0)
				{
					colliderRecipe NewColliderRecipe{};

					for (auto& CollisionObject : Tile.getObjectgroup().getObjects())
					{
						vec2 Position = vec2{ (float)CollisionObject.getPosition().x, (float)CollisionObject.getPosition().y }; // Relative to TL

						// Parse polygon colliders:
						colliderRecipe::polygon NewPolygon{};
						for (auto& Vertex : CollisionObject.getPolygons())
						{
							vec2 NewVertex{ (float)Vertex.x, (float)Vertex.y };
							NewVertex += Position - 0.5f * ImageSize; // Relative to centre
							NewVertex.x /= ImageSize.x; // As proportion of image
							NewVertex.y /= ImageSize.y;
							NewPolygon.Vertices.push_back(NewVertex);
						}
						assert(NewPolygon.Vertices.size() <= B2_MAX_POLYGON_VERTICES); // ** 
						if (NewPolygon.Vertices.size() > 0) NewColliderRecipe.Polygons.push_back(NewPolygon);

						// Parse circle colliders:
						if (CollisionObject.isEllipse())
						{
							vec2 NewCircleSize { (float)CollisionObject.getSize().x, (float)CollisionObject.getSize().x }; // ** Only uses x dimension
							vec2 Centre{ Position + 0.5f * NewCircleSize - 0.5f * ImageSize }; // Relative to centre
							Centre.x /= ImageSize.x; // As proportion of image
							Centre.y /= ImageSize.y;
							float Radius{ b2AbsFloat(0.5f * NewCircleSize.x / ImageSize.x) };

							colliderRecipe::circle NewCircle
							{
								.Position = Centre,
								.Radius = Radius
							};
							NewColliderRecipe.Circles.push_back(NewCircle);
						}
					}

					assert(!NewLevelRecipe.ColliderRecipes.contains(TileFilename)); // Assumes no repeat images with multiple custom colliders
					NewLevelRecipe.ColliderRecipes.insert({ TileFilename, NewColliderRecipe });
				}
			}
		}

		// ============================================================================
		// LAYERS
		// ============================================================================

		for (auto& Layer : LevelData->getLayers())
		{
			vec2 LayerOffset{ (float)Layer.getOffset().x, (float)Layer.getOffset().y };
			vec2 LayerParallaxFactor{ (float)Layer.getParallax().x, (float)Layer.getParallax().y };

			// ============================================================================
			if (Layer.getType() == tson::LayerType::ObjectGroup)
			{
				bool bIsGameLayer{ Layer.getName() == "game"}; // **
				bool bIsGroundTopperLayer{ Layer.getName() == "ground-topper" }; // **
				bool bIsUiLayer{ Layer.getName() == "ui" }; // **

				for (auto& Object : Layer.getObjects())
				{
					// ============================================================================
					if (Object.getObjectType() == tson::ObjectType::Object || Object.getObjectType() == tson::ObjectType::Rectangle)
					{
						objectRecipe NewObjectRecipe{};

						NewObjectRecipe.Id = Object.getId();

						NewObjectRecipe.Name = Object.getName();
						NewObjectRecipe.Class = Object.getType();
						if (Object.getObjectType() != tson::ObjectType::Rectangle)
						{   // CODE JAM HACKY:
							NewObjectRecipe.Filename = ObjectTiles.at(Object.getGid() & TILE_ID_MASK);
						}

						NewObjectRecipe.Position = vec2{ (float)Object.getPosition().x, (float)Object.getPosition().y };
						NewObjectRecipe.Position += LayerOffset;
						NewObjectRecipe.Position.x /= TILE_WIDTH;
						NewObjectRecipe.Position.y /= TILE_HEIGHT;

						NewObjectRecipe.Velocity = ZERO;

						NewObjectRecipe.Size = vec2{ (float)Object.getSize().x, (float)Object.getSize().y };
						NewObjectRecipe.Size.x /= TILE_WIDTH;
						NewObjectRecipe.Size.y /= TILE_HEIGHT;

						if (Object.getObjectType() == tson::ObjectType::Rectangle)
						{   // CODE JAM HACKY:
							NewObjectRecipe.Position += 0.5f * NewObjectRecipe.Size; // Move pos from top left to centre of object [NOT NEEDED FOR OBJECTS, AS SETTING CHANGE IN TILESET]
						}
						NewObjectRecipe.Position -= 0.5f * NewLevelRecipe.Size; // Set origin to centre of map

						NewObjectRecipe.bHorizontalFlip = ((unsigned)Object.getFlipFlags() & FLIPPED_HORIZONTALLY_FLAG) != 0;

						NewObjectRecipe.Rotation = 0.0f; // If use this, will need to convert rad to degrees
						tson::PropertyCollection Properties = Object.getProperties();
						if (Properties.hasProperty("Pivot"))
						{
							tson::Property* Prop = Properties.getProperty("Pivot");
							uint32_t PivotObjectId = Prop->getValue<uint32_t>();
							NewObjectRecipe.PivotPoiId = (int)PivotObjectId;
						}
						else
						{
							NewObjectRecipe.PivotPoiId = -1;
						}

						if (Properties.hasProperty("Value"))
						{
							tson::Property* Prop = Properties.getProperty("Value");
							int ObjectValue = Prop->getValue<int>();
							NewObjectRecipe.Value = (int)ObjectValue;
						}

						NewObjectRecipe.ParallaxFactor = LayerParallaxFactor;
						NewObjectRecipe.MovementRecipes = ParseMovementsProperty(GetProperty<std::string>(Object, "Movements"));

						if (bIsGameLayer)
						{
							bHaveProcessedGameLayer = true;
							NewLevelRecipe.ObjectRecipes.push_back(NewObjectRecipe);
						}
						else if (bIsUiLayer)
						{
							if (NewObjectRecipe.Name == "CAMERA_START")
							{
 								NewLevelRecipe.CameraStartWorldPositionSize = posSize{ NewObjectRecipe.Position, NewObjectRecipe.Size };
							}
						}
						else
						{
							NewObjectRecipe.Class = "STAGE"; // **
							if (!bHaveProcessedGameLayer)
							{
								NewLevelRecipe.BackgroundObjectRecipes.push_back(NewObjectRecipe);
							}
							else
							{
								NewLevelRecipe.ForegroundObjectRecipes.push_back(NewObjectRecipe);
							}
						}
					}
					// ============================================================================
					else if (Object.getObjectType() == tson::ObjectType::Polygon)
					{
						platformRecipe NewPlatformRecipe{};

						NewPlatformRecipe.Id = Object.getId();
						NewPlatformRecipe.Name = Filename + "Platform" + std::to_string(PlatformCounter);
						NewPlatformRecipe.Class = Object.getClassType();

						if (NewPlatformRecipe.Class == "") NewPlatformRecipe.Class = "GROUND"; // **
						if (bIsGroundTopperLayer) NewPlatformRecipe.bIsGroundTopper = true; // **

						vec2 Position = vec2{ (float)Object.getPosition().x, (float)Object.getPosition().y };
						for (auto& Vertex : Object.getPolygons())
						{
							vec2 NewVertex = vec2{ (float)Vertex.x, (float)Vertex.y };
							NewVertex += Position;
							NewVertex += LayerOffset;
							NewVertex.x /= TILE_WIDTH;
							NewVertex.y /= TILE_HEIGHT;
							NewVertex -= 0.5f * NewLevelRecipe.Size; // Set origin to centre of map

							NewPlatformRecipe.Vertices.push_back(NewVertex);
						}

						if (bIsGameLayer || bIsGroundTopperLayer)
						{
							NewLevelRecipe.PlatformRecipes.push_back(NewPlatformRecipe);
							bHaveProcessedGameLayer = true;
						}
						else
						{
//							std::cout << "Unexpected polygon" << '\n';
						}

						++PlatformCounter;
					}
					// ============================================================================
					else if (Object.getObjectType() == tson::ObjectType::Point)
					{
						poiRecipe NewPoiRecipe{};

						NewPoiRecipe.Id = Object.getId();
						NewPoiRecipe.Name = Object.getName();
						NewPoiRecipe.Class = Object.getType();

						NewPoiRecipe.Position = vec2{ (float)Object.getPosition().x, (float)Object.getPosition().y };
						NewPoiRecipe.Position += LayerOffset;
						NewPoiRecipe.Position.x /= TILE_WIDTH;
						NewPoiRecipe.Position.y /= TILE_HEIGHT;
						NewPoiRecipe.Position -= 0.5f * NewLevelRecipe.Size; // Set origin to centre of map
		
						NewLevelRecipe.PoiRecipes.push_back(NewPoiRecipe);
					}
					// ============================================================================
					else
					{
//						std::cout << "Invalid object type: " << Object.getName().c_str() << '\n';
					}
					// ============================================================================
				}
			}
			else
			{
//				std::cout << "Invalid layer type" << std::to_string((int)Layer.getType()) << '\n';
			}
		}

		return NewLevelRecipe;
	}

	// ============================================================================
	template <typename T>
	T GetProperty(tson::Object Obj, std::string PropName)
	{
		T PropValue{};

		tson::PropertyCollection Properties = Obj.getProperties();
		if (Properties.hasProperty(PropName))
		{
			try
			{
				tson::Property* Prop = Properties.getProperty(PropName);
				std::any RawPropValue = Prop->getValue();
				PropValue = std::any_cast<T>(RawPropValue);
			}
			catch (const std::exception& e)
			{
//				std::cout << std::format("Error reading object property: {}", PropName.c_str()) << '\n';
			}
		}

		return PropValue;
	}

	// ============================================================================
	std::vector<movementRecipe> ParseMovementsProperty(std::string PropValue)
	{
		std::vector<movementRecipe> MovementRecipes;
		std::regex Regex{ R"(\{\s*\{\s*([+-]?\d*\.?\d+)\s*,\s*([+-]?\d*\.?\d+)\s*\},\s*([+-]?\d*\.?\d+)\s*,\s*([+-]?\d*\.?\d+)\s*,\s*<([^>]+)>\s*,\s*(.*?)\s*\})" }; // Ouch :(
		std::smatch Match;

		std::string::const_iterator searchStart(PropValue.cbegin());
		while (std::regex_search(searchStart, PropValue.cend(), Match, Regex))
		{
			movementRecipe Movement;
			Movement.Move.x = std::stof(Match[1]);
			Movement.Move.y = std::stof(Match[2]);
			Movement.Rotation = std::stof(Match[3]) * PI; // ** Express rotation in "PI radians"
			Movement.Time = std::stof(Match[4]);
			Movement.EasingId = StringToEasing(Match[5]);
			Movement.ActionsAtEndOfMove = Match[6];
			MovementRecipes.push_back(Movement);
			searchStart = Match.suffix().first;
		}

		return MovementRecipes;
	}

	// ============================================================================

};
