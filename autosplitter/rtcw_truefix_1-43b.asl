// ------------------------------------------------------------
// Return to Castle Wolfenstein - TrueFix Autosplitter
// TrueFix telemetry ABI v1
// ------------------------------------------------------------
//
// speedrunState_t:
//
// +0x00  uint magic
// +0x04  uint version
// +0x08  uint flags
// +0x0C  uint mapSequence
// +0x10  uint transitionSequence
// +0x14  char mapName[32]
//
// ------------------------------------------------------------


state("WolfSP")
{
    uint sr_magic              : "WolfSP.exe", 0x114CF0;
    uint sr_version            : "WolfSP.exe", 0x114CF4;
    uint sr_flags              : "WolfSP.exe", 0x114CF8;
    uint sr_mapSequence        : "WolfSP.exe", 0x114CFC;
    uint sr_transitionSequence : "WolfSP.exe", 0x114D00;
    string32 sr_mapName        : "WolfSP.exe", 0x114D04;
}


// ------------------------------------------------------------
// STARTUP
// ------------------------------------------------------------


startup
{
    // TrueFix telemetry ABI
    vars.SR_MAGIC   = 0x52544357u;
    vars.SR_VERSION = 1u;

    vars.SR_LEVEL_TRANSITION = 1u;   // bit 0
    vars.SR_LOADING          = 2u;   // bit 1
    vars.SR_CUTSCENE         = 4u;   // bit 2
    vars.SR_PLAYER_FROZEN    = 8u;   // bit 3
    vars.SR_CONTROL_LOCKED   = 16u;  // bit 4

    vars.justStarted = false;
    vars.initialLoadSeen = false;


    // --------------------------------------------------------
    // Maps
    // --------------------------------------------------------

    vars.mapListChapter1 = new List<string>
    {
        "escape1",
        "escape2",
        "tram",
        "village1",
        "crypt1",
        "crypt2",
        "church",
        "boss1"
    };

    vars.mapListChapter2 = new List<string>
    {
        "forest",
        "rocket",
        "baseout",
        "assault"
    };

    vars.mapListChapter3 = new List<string>
    {
        "sfm",
        "factory",
        "trainyard",
        "swf"
    };

    vars.mapListChapter4 = new List<string>
    {
        "norway",
        "xlabs",
        "boss2"
    };

    vars.mapListChapter5 = new List<string>
    {
        "dam",
        "village2",
        "chateau",
        "dark",
        "dig",
        "castle",
        "end"
    };


    vars.gameplayMaps = new HashSet<string>();

    foreach (var map in vars.mapListChapter1)
        vars.gameplayMaps.Add(map);

    foreach (var map in vars.mapListChapter2)
        vars.gameplayMaps.Add(map);

    foreach (var map in vars.mapListChapter3)
        vars.gameplayMaps.Add(map);

    foreach (var map in vars.mapListChapter4)
        vars.gameplayMaps.Add(map);

    foreach (var map in vars.mapListChapter5)
        vars.gameplayMaps.Add(map);


    // --------------------------------------------------------
    // Names
    // --------------------------------------------------------

    vars.chapterNames = new List<string>
    {
        "Ominous Rumors + Dark Secret",
        "Weapons of Vengeance",
        "Deadly Designs",
        "Deathshead's Playground",
        "Return Engagement + Operation Resurrection"
    };

    vars.individualNames1 = new List<string>
    {
        "Escape!",
        "Castle Keep",
        "Tram Ride",
        "Village",
        "Catacombs",
        "Crypt",
        "The Defiled Church",
        "Tomb"
    };

    vars.individualNames2 = new List<string>
    {
        "Forest Compound",
        "Rocket Base",
        "Radar Installation",
        "Air Base Assault"
    };

    vars.individualNames3 = new List<string>
    {
        "Kugelstadt",
        "The Bombed Factory",
        "The Trainyards",
        "Secret Weapons Facility"
    };

    vars.individualNames4 = new List<string>
    {
        "Ice Station Norway",
        "X-Labs",
        "Super Soldier"
    };

    vars.individualNames5 = new List<string>
    {
        "Bramburg Dam",
        "Paderborn Village",
        "Chateau Schufstaffel",
        "Unhallowed Ground",
        "The Dig",
        "Return to Castle Wolfenstein",
        "Heinrich"
    };


    // --------------------------------------------------------
    // Settings
    // --------------------------------------------------------

    settings.Add("cat_all", true, "Full Game");

    settings.Add("chaptersOnly", false, "Chapters");

    for (int chapter = 1; chapter <= 5; chapter++)
    {
        settings.Add(
            "cat_chap" + chapter,
            false,
            vars.chapterNames[chapter - 1],
            "chaptersOnly"
        );
    }


    for (int chapter = 1; chapter <= 5; chapter++)
    {
        settings.Add(
            "individualLevelsC" + chapter,
            false,
            "Chapter " + chapter + " Individual Levels"
        );

        var names =
            chapter == 1 ? vars.individualNames1 :
            chapter == 2 ? vars.individualNames2 :
            chapter == 3 ? vars.individualNames3 :
            chapter == 4 ? vars.individualNames4 :
                           vars.individualNames5;

        for (int i = 0; i < names.Count; i++)
        {
            settings.Add(
                "miss" + (i + 1) + "_chap_" + chapter,
                false,
                names[i],
                "individualLevelsC" + chapter
            );
        }
    }


    // Runtime state
    vars.running = false;
    vars.cutsceneCount = 0;
    vars.secondCutsceneStarted = false;

    refreshRate = 240;
}


// ------------------------------------------------------------
// INIT
// ------------------------------------------------------------

init
{
    if (current.sr_magic != vars.SR_MAGIC)
    {
        vars.running = false;
        return false;
    }

    if (current.sr_version != vars.SR_VERSION)
    {
        vars.running = false;
        return false;
    }

    vars.running = true;

    vars.justStarted = false;
    vars.initialLoadSeen = false;

    vars.cutsceneCount = 0;
    vars.secondCutsceneStarted = false;
}


// ------------------------------------------------------------
// ON START
// ------------------------------------------------------------

onStart
{
    if (settings["cat_all"])
    {
        vars.justStarted = false;
        vars.initialLoadSeen = false;
        timer.IsGameTimePaused = false;
    }
    else
    {
        vars.justStarted = true;
        vars.initialLoadSeen = false;
        timer.IsGameTimePaused = true;
    }
}


// ------------------------------------------------------------
// EXIT
// ------------------------------------------------------------

exit
{
    timer.IsGameTimePaused = true;
    vars.running = false;
}


// ------------------------------------------------------------
// SHUTDOWN
// ------------------------------------------------------------

shutdown
{
    timer.IsGameTimePaused = true;
    vars.running = false;
}


// ------------------------------------------------------------
// UPDATE
//
// Keeps track of cutscene starts.
//
// Certain RTCW category endings happen at the SECOND cutscene
// on a map.
// ------------------------------------------------------------

update
{
	


    if (!vars.running)
        return;

    bool loading =
        (current.sr_flags & vars.SR_LOADING) != 0;

    if (vars.justStarted)
    {
        // We have now seen the expected initial load begin.
        if (loading)
        {
            vars.initialLoadSeen = true;
        }

        // Do not release the timer until that load has
        // actually completed.
        if (vars.initialLoadSeen && !loading)
        {
            vars.justStarted = false;
        }
    }


    vars.secondCutsceneStarted = false;


    bool mapChanged =
        current.sr_mapSequence !=
        old.sr_mapSequence;


    if (mapChanged)
    {
        vars.cutsceneCount = 0;
    }


    bool cutsceneStarted =
        (current.sr_flags & vars.SR_CUTSCENE) != 0 &&
        (old.sr_flags & vars.SR_CUTSCENE) == 0;


    if (cutsceneStarted)
    {
        vars.cutsceneCount++;

        if (vars.cutsceneCount == 2)
        {
            vars.secondCutsceneStarted = true;
        }
    }
}


// ------------------------------------------------------------
// START
// ------------------------------------------------------------

start
{
    if (!vars.running)
        return false;


    bool cutsceneStarted =
        (current.sr_flags & vars.SR_CUTSCENE) != 0 &&
        (old.sr_flags & vars.SR_CUTSCENE) == 0;


    bool mapChanged =
        current.sr_mapSequence !=
        old.sr_mapSequence;


    // --------------------------------------------------------
    // Full Game
    //
    // Start when cutscene1 begins.
    // --------------------------------------------------------

    if (settings["cat_all"] &&
        current.sr_mapName == "cutscene1" &&
        cutsceneStarted)
    {
        return true;
    }


    // --------------------------------------------------------
    // Chapters and Individual Levels
    //
    // Start when the selected first gameplay BSP becomes active.
    // --------------------------------------------------------

    if (!mapChanged)
        return false;


    for (int chapter = 1; chapter <= 5; chapter++)
    {
        var maps =
            chapter == 1 ? vars.mapListChapter1 :
            chapter == 2 ? vars.mapListChapter2 :
            chapter == 3 ? vars.mapListChapter3 :
            chapter == 4 ? vars.mapListChapter4 :
                           vars.mapListChapter5;


        for (int i = 0; i < maps.Count; i++)
        {
            bool chapterStart =
                settings["cat_chap" + chapter] &&
                i == 0;


            bool individualStart =
                settings[
                    "miss" + (i + 1) +
                    "_chap_" + chapter
                ];


            if ((chapterStart || individualStart) &&
                current.sr_mapName == maps[i])
            {
                return true;
            }
        }
    }


    return false;
}


// ------------------------------------------------------------
// SPLIT
// ------------------------------------------------------------

split
{
    if (!vars.running)
        return false;

    bool mapChanged =
        current.sr_mapSequence !=
        old.sr_mapSequence;

    bool transitionTriggered =
        current.sr_transitionSequence !=
        old.sr_transitionSequence;

    bool levelTransition =
        (current.sr_flags &
         vars.SR_LEVEL_TRANSITION) != 0;

    bool playerFrozen =
        (current.sr_flags &
         vars.SR_PLAYER_FROZEN) != 0;

    bool cutsceneStarted =
        (current.sr_flags & vars.SR_CUTSCENE) != 0 &&
        (old.sr_flags & vars.SR_CUTSCENE) == 0;


    // Normal Individual Level endpoint:
    //
    // - changelevel has been committed
    // - level transition state is active
    // - player has lost control / is PM_FREEZE
    bool normalILEnd = transitionTriggered && levelTransition;


    // ========================================================
    // SPECIAL FINISHES
    // ========================================================


    // --------------------------------------------------------
    // FOREST / ASSAULT
    //
    // These levels finish when their end sequence starts.
    // They enter CUTSCENE state instead of immediately using
    // the normal ChangeLevel transition.
    // --------------------------------------------------------

    if (settings["miss1_chap_2"] && current.sr_mapName == "forest" && cutsceneStarted)
    {
        return true;
    }

    if (settings["miss4_chap_2"] && current.sr_mapName == "assault" && cutsceneStarted)
    {
        return true;
    }

    if (settings["miss4_chap_3"] && current.sr_mapName == "swf" && cutsceneStarted)
    {
        return true;
    }


    // --------------------------------------------------------
    // FINAL BOSS / END
    //
    // Full Game
    // Chapter 5
    // Heinrich Individual Level
    //
    // Finish on the SECOND cutscene after the boss fight.
    // --------------------------------------------------------

    if (current.sr_mapName == "end" && vars.secondCutsceneStarted &&
        (
            settings["cat_all"] ||
            settings["cat_chap5"] ||
            settings["miss7_chap_5"]
        ))
    {
        return true;
    }


    // --------------------------------------------------------
    // BOSS1
    //
    // Chapter 1 and Tomb IL finish on second cutscene.
    //
    // Full Game does NOT finish here.
    // --------------------------------------------------------

    if (current.sr_mapName == "boss1" && vars.secondCutsceneStarted &&
        (
            settings["cat_chap1"] ||
            settings["miss8_chap_1"]
        ))
    {
        return true;
    }


    // --------------------------------------------------------
    // SWF
    //
    // Chapter 3 finish.
    // --------------------------------------------------------

    if (current.sr_mapName == "swf" && vars.secondCutsceneStarted &&
        settings["cat_chap3"])
    {
        return true;
    }


    // ========================================================
    // FULL GAME
    // ========================================================
    //
    // Full-game splits happen when the NEXT gameplay BSP
    // actually becomes active.
    //
    // Do not split when entering escape1 because the timer
    // already started during cutscene1.
    //
    // Do not use map change for the final "end" finish;
    // that is handled above by the second-cutscene rule.
    // ========================================================

    if (settings["cat_all"] &&
        mapChanged &&
        current.sr_mapName != "escape1" &&
        vars.gameplayMaps.Contains(current.sr_mapName))
    {
        return true;
    }


    // ========================================================
    // CHAPTER RUNS
    // ========================================================
    //
    // Split whenever another gameplay level inside the selected
    // chapter becomes active.
    //
    // The final finish for each chapter is handled separately:
    //
    // Chapter 1 -> boss1 second cutscene
    // Chapter 2 -> assault second cutscene
    // Chapter 3 -> swf second cutscene
    // Chapter 4 -> Boss2 special rule
    // Chapter 5 -> end second cutscene
    // ========================================================

    if (mapChanged)
    {
        for (int chapter = 1; chapter <= 5; chapter++)
        {
            if (!settings["cat_chap" + chapter])
                continue;


            var maps =
                chapter == 1 ? vars.mapListChapter1 :
                chapter == 2 ? vars.mapListChapter2 :
                chapter == 3 ? vars.mapListChapter3 :
                chapter == 4 ? vars.mapListChapter4 :
                               vars.mapListChapter5;


            // Index 0 is the map that started the chapter.
            // Do not split on the starting map.
            for (int i = 1; i < maps.Count; i++)
            {
                if (current.sr_mapName == maps[i])
                {
                    return true;
                }
            }
        }
    }


    // ========================================================
    // INDIVIDUAL LEVEL RUNS
    // ========================================================

    for (int chapter = 1; chapter <= 5; chapter++)
    {
        var maps =
            chapter == 1 ? vars.mapListChapter1 :
            chapter == 2 ? vars.mapListChapter2 :
            chapter == 3 ? vars.mapListChapter3 :
            chapter == 4 ? vars.mapListChapter4 :
                           vars.mapListChapter5;


        for (int i = 0; i < maps.Count; i++)
        {
            bool selected =
                settings[
                    "miss" + (i + 1) +
                    "_chap_" + chapter
                ];


            if (!selected)
                continue;


            string map = maps[i];


            if (current.sr_mapName != map)
                continue;


            // These maps have special ending rules.
            if (map == "forest")
                continue;

            if (map == "assault")
                continue;

            if (map == "swf")
                continue;

            if (map == "boss1")
                continue;

            if (map == "end")
                continue;


            // Normal IL finish:
            // player has reached an accepted changelevel endpoint
            // and has lost control.
            if (normalILEnd)
            {
                return true;
            }
        }
    }


    return false;
}


// ------------------------------------------------------------
// RESET
//
// Manual reset for now.
// ------------------------------------------------------------

reset
{
    return false;
}


// ------------------------------------------------------------
// LOAD REMOVAL
//
// TrueFix controls this directly.
//
// Same-map quickloads intentionally do NOT set SR_LOADING.
// ------------------------------------------------------------

isLoading
{
    if (!vars.running)
        return true;

    bool loading =
        (current.sr_flags & vars.SR_LOADING) != 0;

    return vars.justStarted || loading;
}