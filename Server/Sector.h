#pragma once


void SetSector(int id);
void CheckMoveSector(int id);
void ChangeSector(int id, bool update = true);
unordered_set<int> MakeNearList(int c_id);
