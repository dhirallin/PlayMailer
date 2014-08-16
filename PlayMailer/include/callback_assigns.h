void AssignPluginCallbacks(PluginCallbacks *callbacks)
{
callbacks->PTR_hSessionSettingsDialog = &hSessionSettingsDialog;
callbacks->PTR_hGameSettingsDialog = &hGameSettingsDialog;
callbacks->PTR_hGGChildDialog = &hGGChildDialog;
callbacks->PTR_NewGameInstance = &NewGameInstance;
callbacks->PTR_settings = &settings;
callbacks->PTR_hMainWnd = &hMainWnd;
callbacks->EDITPLAYERS_BUTTON = IDC_EDITPLAYERS_BUTTON;
callbacks->PTR_MouseModifier = &MouseModifier;
callbacks->PTR_MouseSpeed = &MouseSpeed;

callbacks->SaveGlobalGameSettings = SaveGlobalGameSettings;
callbacks->SaveSessionList = SaveSessionList;
callbacks->cfgSetInt = cfgSetInt;
callbacks->cfgGetInt = cfgGetInt;
callbacks->cfgGetBool = cfgGetBool;
callbacks->cfgSetBool = cfgSetBool;
callbacks->cfgGetString = cfgGetString;
callbacks->cfgSetString = cfgSetString;
callbacks->cfgSetInt64 = cfgSetInt64;
callbacks->cfgGetInt64 = cfgGetInt64;
callbacks->DLUToPixelsX = DLUToPixelsX;
callbacks->DLUToPixelsY = DLUToPixelsY;
callbacks->PressStringKeys = PressStringKeys;
callbacks->WaitForSaveFile = WaitForSaveFile;
callbacks->WaitForSaveFileThread = WaitForSaveFileThread;
callbacks->StartSaveFileThread = StartSaveFileThread;
callbacks->ExportSaveFile = ExportSaveFile;
callbacks->ImportSaveFile = ImportSaveFile;
callbacks->PressKey = PressKey;
callbacks->PressHotKey = PressHotKey;
callbacks->MoveMouse = MoveMouse;
callbacks->MoveMouseMod = MoveMouseMod;
callbacks->ResetMouse = ResetMouse;
callbacks->ResetMouseVert = ResetMouseVert;
callbacks->ResetMouseHoriz = ResetMouseHoriz;
callbacks->SetMouseSpeed = SetMouseSpeed;
callbacks->PressLButton = PressLButton;
callbacks->PressLButtonDown = PressLButtonDown;
callbacks->PressLButtonUp = PressLButtonUp;
callbacks->CopyPlayers = CopyPlayers;
callbacks->FreePlayers = FreePlayers;
callbacks->GetNumFactions = GetNumFactions;
callbacks->GetDOSBoxPath = GetDOSBoxPath;
callbacks->GetDOSBoxConfPath = GetDOSBoxConfPath;
callbacks->ReplaceLinesInFile = ReplaceLinesInFile;
callbacks->AllocPlayers = AllocPlayers;
callbacks->GetFolderSelection = GetFolderSelection;
callbacks->GetFileSelection = GetFileSelection;
callbacks->trimWhiteSpace = trimWhiteSpace;
callbacks->MessageBoxS = MessageBoxS;
callbacks->DialogBoxParamS = DialogBoxParamS;
callbacks->CheckFullScreen = CheckFullScreen;
callbacks->_LoadGame = _LoadGame;
callbacks->isYourTurn = isYourTurn;
callbacks->GetNextPlayerIndex = GetNextPlayerIndex;
callbacks->mod = mod;
callbacks->LL_FreeAll = LL_FreeAll;
callbacks->LL_Add = LL_Add;
callbacks->LL_Size = LL_Size;
callbacks->LL_GetItemIndex = LL_GetItemIndex;
callbacks->getYourPlayerName = getYourPlayerName;
callbacks->getCurrentPlayerTeam = getCurrentPlayerTeam;
callbacks->getCurrentPlayerFaction = getCurrentPlayerFaction;
callbacks->getCurrentPlayerFactionName = getCurrentPlayerFactionName;
callbacks->FreeTeams = FreeTeams;
callbacks->AllocTeams = AllocTeams;
callbacks->CopyTeams = CopyTeams;
callbacks->BuildTeamList = BuildTeamList;
callbacks->FindTeam = FindTeam;
callbacks->SetTopMost = SetTopMost;
callbacks->CreateDialogFont = CreateDialogFont;
callbacks->SleepC = SleepC;
callbacks->CreateSchTask = CreateSchTask;
callbacks->PressKeyDown = PressKeyDown;
callbacks->PressKeyUp = PressKeyUp;
callbacks->SpinUpCDDrive = SpinUpCDDrive;
}

