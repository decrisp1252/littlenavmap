/*****************************************************************************
* Copyright 2015-2020 Alexander Barthel alex@littlenavmap.org
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/

#include "common/jsoninfobuilder.h"

// Use JSON library
#include "json/nlohmann/json.hpp"
using JSON = nlohmann::json;

JsonInfoBuilder::JsonInfoBuilder(QObject *parent)
  : AbstractInfoBuilder(parent)
{
  contentTypeHeader = "application/json";
}

JsonInfoBuilder::~JsonInfoBuilder()
{

}

QByteArray JsonInfoBuilder::airport(AirportInfoData airportInfoData) const
{

    AirportInfoData& data = airportInfoData;

    JSON json = {
        { "ident", qUtf8Printable(data.airport.ident) },
        { "icao", qUtf8Printable(data.airport.icao) },
        { "name", qUtf8Printable(data.airport.name) },
        { "region", qUtf8Printable(data.airport.region) },
        { "closed", data.airport.closed() },
        { "elevation", qUtf8Printable(Unit::altFeet(data.airport.getPosition().getAltitude())) },
        { "magneticDeclination", qUtf8Printable(map::magvarText(data.airport.magvar)) },
        { "position", nullptr },
        { "rating", nullptr },
        { "iata", nullptr },
        { "city", nullptr },
        { "state", nullptr },
        { "country", nullptr },
        { "transitionAltitude", nullptr },
        { "facilities", JSON::array()},
        { "runways", JSON::array()},
        { "parking", JSON::object()},
        { "longestRunwayLength", nullptr },
        { "longestRunwayHeading", nullptr },
        { "longestRunwaySurface", nullptr },
        { "metar", JSON::object()},
        { "sunrise", nullptr},
        { "sunset", nullptr},
        { "activeDateTime", nullptr},
        { "activeDateTimeSource", nullptr},
        { "com", JSON::object()},
    };

    /* Null fields populated if available */

    if(data.airportInformation!=nullptr){
        json["rating"] = data.airportInformation->valueInt("rating");
        json["iata"] = qUtf8Printable(data.airportInformation->valueStr("iata"));
        json["position"] = qUtf8Printable(getCoordinatesString(data.airportInformation));

        if(data.airportInformation->valueInt("num_parking_gate") > 0){
            json["parking"].push_back({ "gates", data.airportInformation->valueInt("num_parking_gate") });
        }
        if(data.airportInformation->valueInt("num_jetway") > 0){
            json["parking"].push_back({ "jetWays", data.airportInformation->valueInt("num_jetway") });
        }
        if(data.airportInformation->valueInt("num_parking_ga_ramp") > 0){
            json["parking"].push_back({ "gaRamps", data.airportInformation->valueInt("num_parking_ga_ramp") });
        }
        if(data.airportInformation->valueInt("num_parking_cargo") > 0){
            json["parking"].push_back({ "cargo", data.airportInformation->valueInt("num_parking_cargo") });
        }
        if(data.airportInformation->valueInt("num_parking_mil_cargo") > 0){
            json["parking"].push_back({ "militaryCargo", data.airportInformation->valueInt("num_parking_mil_cargo") });
        }
        if(data.airportInformation->valueInt("num_parking_mil_combat") > 0){
            json["parking"].push_back({ "militaryCombat", data.airportInformation->valueInt("num_parking_mil_combat") });
        }
        if(data.airportInformation->valueInt("num_helipad") > 0){
            json["parking"].push_back({ "helipads", data.airportInformation->valueInt("num_helipad") });
        }

    }

    if(data.airportAdminNames!=nullptr){
        json["city"] = qUtf8Printable(data.airportAdminNames->city);
        json["state"] = qUtf8Printable(data.airportAdminNames->state);
        json["country"] = qUtf8Printable(data.airportAdminNames->country);
    }

    if(data.transitionAltitude!=nullptr && *data.transitionAltitude > 0){
        json["transitionAltitude"] = qUtf8Printable(Unit::altFeet(*data.transitionAltitude));
    }

    if(!data.airport.noRunways()){
        json["longestRunwayLength"] = qUtf8Printable(Unit::distShortFeet(data.airport.longestRunwayLength));
        if(data.airportInformation!=nullptr){
            json["longestRunwayWidth"] = qUtf8Printable(Unit::distShortFeet(data.airportInformation->valueInt("longest_runway_width")));
            json["longestRunwayHeading"] = qUtf8Printable(getHeadingsStringByMagVar(data.airportInformation->valueFloat("longest_runway_heading"),data.airport.magvar));
            json["longestRunwaySurface"] = qUtf8Printable(data.airportInformation->valueStr("longest_runway_surface"));
        }
    }

    if(data.sunrise!= nullptr){
        json["sunrise"]=qUtf8Printable(data.sunrise->toString());
    }
    if(data.sunset!= nullptr){
        json["sunset"]=qUtf8Printable(data.sunset->toString());
    }

    if(data.activeDateTime!=nullptr){
        json["activeDateTime"]=qUtf8Printable(data.activeDateTime->toString());
    }
    if(data.activeDateTimeSource!=nullptr){
        json["activeDateTimeSource"]=qUtf8Printable(*data.activeDateTimeSource);
    }

    /* METAR */

    // Simulator
    if(!data.weatherContext.fsMetar.isEmpty()){
        json["metar"].push_back({ "simulator", {
                {"station", qUtf8Printable(data.weatherContext.fsMetar.metarForStation)},
                {"nearest", qUtf8Printable(data.weatherContext.fsMetar.metarForNearest)},
                {"interpolated", qUtf8Printable(data.weatherContext.fsMetar.metarForInterpolated)},
            }
        });
    }

    // Active Sky
    if(!data.weatherContext.asMetar.isEmpty()){
        json["metar"].push_back({ "activesky", {
                {"station", qUtf8Printable(data.weatherContext.asMetar)},
            }
        });
    }

    // NOAA
    if(
       !data.weatherContext.noaaMetar.metarForStation.isEmpty() ||
       !data.weatherContext.noaaMetar.metarForNearest.isEmpty()
      ){
        json["metar"].push_back({ "noaa", {
                {"station", qUtf8Printable(data.weatherContext.noaaMetar.metarForStation)},
                {"nearest", qUtf8Printable(data.weatherContext.noaaMetar.metarForNearest)},
            }
        });
    }

    // VATSIM
    if(
       !data.weatherContext.vatsimMetar.metarForStation.isEmpty() ||
       !data.weatherContext.vatsimMetar.metarForNearest.isEmpty()
      ){
        json["metar"].push_back({ "vatsim", {
                {"station", qUtf8Printable(data.weatherContext.vatsimMetar.metarForStation)},
                {"nearest", qUtf8Printable(data.weatherContext.vatsimMetar.metarForNearest)},
            }
        });
    }

    // IVAO
    if(
       !data.weatherContext.ivaoMetar.metarForStation.isEmpty() ||
       !data.weatherContext.ivaoMetar.metarForStation.isEmpty()
      ){
        json["metar"].push_back({"ivao", {
                {"station", qUtf8Printable(data.weatherContext.ivaoMetar.metarForStation)},
                {"nearest", qUtf8Printable(data.weatherContext.ivaoMetar.metarForNearest)},
            }
        });
    }

    /* COM */

    if(data.airport.towerFrequency > 0){
        json["com"].push_back({"Tower:", qUtf8Printable(formatComFrequency(data.airport.towerFrequency))});
    }
    if(data.airport.atisFrequency > 0){
        json["com"].push_back({"ATIS:", qUtf8Printable(formatComFrequency(data.airport.atisFrequency))});
    }
    if(data.airport.awosFrequency > 0){
        json["com"].push_back({"AWOS:", qUtf8Printable(formatComFrequency(data.airport.awosFrequency))});
    }
    if(data.airport.asosFrequency > 0){
        json["com"].push_back({"ASOS:", qUtf8Printable(formatComFrequency(data.airport.asosFrequency))});
    }
    if(data.airport.unicomFrequency > 0){
        json["com"].push_back({"UNICOM:", qUtf8Printable(formatComFrequency(data.airport.unicomFrequency))});
    }

    /* Facilities */

    if(data.airport.closed())
        json["facilities"].push_back(qUtf8Printable(tr("Closed")));
    if(data.airport.addon())
        json["facilities"].push_back(qUtf8Printable(tr("Add-on")));
    if(data.airport.is3d())
        json["facilities"].push_back(qUtf8Printable(tr("3D")));
    if(data.airport.flags.testFlag(map::MapAirportFlag::AP_MIL))
        json["facilities"].push_back(qUtf8Printable(tr("Military")));
    if(data.airport.apron())
        json["facilities"].push_back(qUtf8Printable(tr("Aprons")));
    if(data.airport.taxiway())
        json["facilities"].push_back(qUtf8Printable(tr("Taxiways")));
    if(data.airport.towerObject())
        json["facilities"].push_back(qUtf8Printable(tr("Tower Object")));
    if(data.airport.parking())
        json["facilities"].push_back(qUtf8Printable(tr("Parking")));
    if(data.airport.helipad())
        json["facilities"].push_back(qUtf8Printable(tr("Helipads")));
    if(data.airport.flags.testFlag(map::MapAirportFlag::AP_AVGAS))
        json["facilities"].push_back(qUtf8Printable(tr("Avgas")));
    if(data.airport.flags.testFlag(map::MapAirportFlag::AP_JETFUEL))
        json["facilities"].push_back(qUtf8Printable(tr("Jetfuel")));
    if(data.airport.flags.testFlag(map::MapAirportFlag::AP_ILS))
        json["facilities"].push_back(qUtf8Printable(tr("ILS")));
    if(data.airport.vasi())
        json["facilities"].push_back(qUtf8Printable(tr("VASI")));
    if(data.airport.als())
        json["facilities"].push_back(qUtf8Printable(tr("ALS")));
    if(data.airport.flatten == 1)
        json["facilities"].push_back(qUtf8Printable(tr("Flatten")));
    if(data.airport.flatten == 0)
        json["facilities"].push_back(qUtf8Printable(tr("No Flatten")));

    /* Runways */

    if(data.airport.hard())
        json["runways"].push_back(qUtf8Printable(tr("Hard")));
    if(data.airport.soft())
        json["runways"].push_back(qUtf8Printable(tr("Soft")));
    if(data.airport.water())
        json["runways"].push_back(qUtf8Printable(tr("Water")));
    if(data.airport.closedRunways())
        json["runways"].push_back(qUtf8Printable(tr("Closed")));
    if(data.airport.flags.testFlag(map::MapAirportFlag::AP_LIGHT))
        json["runways"].push_back(qUtf8Printable(tr("Lighted")));

    /* Fields added only if available */

    if(data.airport.xpident.length() > 0 && data.airport.ident != data.airport.xpident)
        json["x-plane-ident"] = qUtf8Printable(data.airport.xpident);

    return json.dump().data();
}

