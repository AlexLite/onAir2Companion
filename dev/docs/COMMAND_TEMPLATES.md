# FDOnAir Command Templates (Companion Module)

Источник: `dev/companion-module/src/index.ts`

Всего шаблонов: 101.

| ID | Command | Category | Args (default) | Description |
| --- | --- | --- | --- | --- |
| `player_time` | `Player.Time` | Player | `?` | Get current player time |
| `player_block` | `Player.Block` | Player | `?` | Get current block playback info |
| `player_block_info` | `Player.BlockInfo` | Player | `0 0 ?` | Get detailed block info by schedule/block number |
| `player_video` | `Player.Video` | Player | `1` | Switch to input video line |
| `player_switch_video` | `Player.SwitchVideo` | Player | `1 2` |  |
| `player_default` | `Player.Default` | Player | `1` |  |
| `player_title` | `Player.Title` | Player | `?` |  |
| `player_title_button` | `Player.TitleButton` | Player | `?` |  |
| `player_title_object` | `Player.TitleObject` | Player | `?` |  |
| `player_title_object_list` | `Player.TitleObjectList` | Player | `?` |  |
| `player_set_title_button` | `Player.SetTitleButton` | Player | `0 file.xml` |  |
| `player_set_title_object` | `Player.SetTitleObject` | Player | `0 file.xml` |  |
| `player_set_title_objects` | `Player.SetTitleObjects` | Player | `list.xml` |  |
| `player_set_title_object_task` | `Player.SetTitleObjectTask` | Player | `0 task.xml` |  |
| `player_reload_title_button` | `Player.ReloadTitleButton` | Player | `0` |  |
| `player_clear_title_button` | `Player.ClearTitleButton` | Player | `0` |  |
| `player_abort_title_button` | `Player.AbortTitleButton` | Player | `0` |  |
| `player_abort_title_object` | `Player.AbortTitleObject` | Player | `0` |  |
| `player_abort_title_objects` | `Player.AbortTitleObjects` | Player | `` |  |
| `player_prepare_video` | `Player.PrepareVideo` | Player | `1 00:00:00.00` |  |
| `player_pause` | `Player.Pause` | Player | `1` | Pause or resume player |
| `player_mute` | `Player.Mute` | Player | `1` |  |
| `player_set_mute` | `Player.SetMute` | Player | `0/1` |  |
| `player_volume` | `Player.Volume` | Player | `?` |  |
| `player_set_volume` | `Player.SetVolume` | Player | `100` |  |
| `player_aux` | `Player.Aux` | Player | `?` |  |
| `player_set_aux` | `Player.SetAux` | Player | `1` |  |
| `player_turn_aux` | `Player.TurnAux` | Player | `0/1` |  |
| `player_titling` | `Player.Titling` | Player | `?` |  |
| `player_set_titling` | `Player.SetTitling` | Player | `0/1` |  |
| `player_turn_titling` | `Player.TurnTitling` | Player | `` |  |
| `player_logotip` | `Player.Logotip` | Player | `?` |  |
| `player_set_logotip` | `Player.SetLogotip` | Player | `0/1` |  |
| `player_turn_logotip` | `Player.TurnLogotip` | Player | `` |  |
| `player_fore_fade` | `Player.ForeFade` | Player | `?` |  |
| `player_set_fore_fade` | `Player.SetForeFade` | Player | `10` |  |
| `player_back_fade` | `Player.BackFade` | Player | `?` |  |
| `player_set_back_fade` | `Player.SetBackFade` | Player | `10` |  |
| `player_sound` | `Player.Sound` | Player | `?` |  |
| `player_sound_file_stop` | `Player.SoundFileStop` | Player | `` |  |
| `player_audio_bound` | `Player.AudioBound` | Player | `?` |  |
| `player_set_audio_bound` | `Player.SetAudioBound` | Player | `0/1` |  |
| `player_peak` | `Player.Peak` | Player | `?` |  |
| `player_line_enabled` | `Player.LineEnabled` | Player | `0 ?` |  |
| `player_set_wait_pause` | `Player.SetWaitPause` | Player | `0/1` |  |
| `player_set_wait_sound` | `Player.SetWaitSound` | Player | `0/1` |  |
| `player_set_wait_video` | `Player.SetWaitVideo` | Player | `0/1` |  |
| `player_set_wait_title` | `Player.SetWaitTitle` | Player | `0/1` |  |
| `player_fore_title_stop` | `Player.ForeTitleStop` | Player | `` |  |
| `player_pure_video` | `Player.PureVideo` | Player | `0/1` |  |
| `shedule_start` | `Shedule.Start` | Shedule | `` | Start current schedule |
| `shedule_pause` | `Shedule.Pause` | Shedule | `0/1` |  |
| `shedule_set_pause` | `Shedule.SetPause` | Shedule | `0/1` |  |
| `shedule_break` | `Shedule.Break` | Shedule | `` |  |
| `shedule_skip_block` | `Shedule.SkipBlock` | Shedule | `0` |  |
| `shedule_active` | `Shedule.ActiveShedule` | Shedule | `0 ?` |  |
| `shedule_run_position` | `Shedule.RunPosition` | Shedule | `0 ?` |  |
| `shedule_set_run_position` | `Shedule.SetRunPosition` | Shedule | `0 1` |  |
| `shedule_show_first` | `Shedule.ShowFirst` | Shedule | `0` |  |
| `shedule_length` | `Shedule.Length` | Shedule | `0 ?` |  |
| `shedule_item` | `Shedule.Item` | Shedule | `0 1 ?` |  |
| `shedule_item_ex` | `Shedule.ItemEx` | Shedule | `0 1 ?` |  |
| `shedule_set_item` | `Shedule.SetItem` | Shedule | `0 1 command...` |  |
| `shedule_add_item` | `Shedule.AddItem` | Shedule | `0 command...` |  |
| `shedule_delete_item` | `Shedule.DeleteItem` | Shedule | `0 1` |  |
| `shedule_delete_item_above` | `Shedule.DeleteItemAbove` | Shedule | `0 1` |  |
| `shedule_erase_list` | `Shedule.EraseList` | Shedule | `0` |  |
| `shedule_save_to_file` | `Shedule.SaveToFile` | Shedule | `0 D:/example.air` |  |
| `shedule_read_from_file` | `Shedule.ReadFromFile` | Shedule | `0 D:/example.air` |  |
| `shedule_insert_from_file` | `Shedule.InsertFromFile` | Shedule | `0 1 D:/example.air` |  |
| `shedule_check_state` | `Shedule.CheckState` | Shedule | `` | Check command states in schedule |
| `shedule_time_shift` | `Shedule.TimeShift` | Shedule | `0 1 -1000` |  |
| `shedule_set_run_shedule` | `Shedule.SetRunShedule` | Shedule | `0/1` |  |
| `shedule_turn_run_shedule` | `Shedule.TurnRunShedule` | Shedule | `` |  |
| `shedule_insert_mode` | `Shedule.InsertMode` | Shedule | `?` |  |
| `shedule_set_insert_mode` | `Shedule.SetInsertMode` | Shedule | `0/1` |  |
| `shedule_edit_position` | `Shedule.EditPosition` | Shedule | `0 ?` |  |
| `shedule_set_edit_position` | `Shedule.SetEditPosition` | Shedule | `0 1` |  |
| `shedule_set_item_name` | `Shedule.SetItemName` | Shedule | `0 1 Name` |  |
| `shedule_set_item_title` | `Shedule.SetItemTitle` | Shedule | `0 1 Title` |  |
| `shedule_set_item_duration` | `Shedule.SetItemDuration` | Shedule | `0 1 00:00:05.00` |  |
| `shedule_set_item_fade` | `Shedule.SetItemFade` | Shedule | `0 1 10` |  |
| `shedule_set_item_wait_type` | `Shedule.SetItemWaitType` | Shedule | `0 1 follow` |  |
| `shedule_set_item_wait_start` | `Shedule.SetItemWaitStart` | Shedule | `0 1 12:00:00.00` |  |
| `shedule_set_item_wait_duration` | `Shedule.SetItemWaitDuration` | Shedule | `0 1 5.00` |  |
| `shedule_set_item_wait_decrease` | `Shedule.SetItemWaitDecrease` | Shedule | `0 1 0/1` |  |
| `shedule_set_item_logo` | `Shedule.SetItemLogo` | Shedule | `0 1 0/1` |  |
| `shedule_set_item_inout` | `Shedule.SetItemInOut` | Shedule | `0 1 in out` |  |
| `shedule_lock_run` | `Shedule.LockRun` | Shedule | `?` |  |
| `shedule_set_lock_run` | `Shedule.SetLockRun` | Shedule | `0/1` |  |
| `shedule_get_next_error_line` | `Shedule.GetNextErrorLine` | Shedule | `0 ?` |  |
| `folder_info` | `Folder.Info` | Folder | `D:/` | Get folder listing/info |
| `folder_file_info` | `Folder.FileInfo` | Folder | `D:/file.avi` | Get file info |
| `folder_refresh` | `Folder.Refresh` | Folder | `D:/` | Refresh folder metadata cache |
| `folder_summary` | `Folder.Summary` | Folder | `D:/` | Get folder summary |
| `setting_name` | `Setting.Name` | Setting | `?` | Get onAir instance name |
| `loading_break` | `Loading.Break` | Loading | `` | Interrupt loading process |
| `mirror_get_info_all` | `OnAir1.MirrorSave.GetInfoAll` | MirrorSave | `` | N+1: request full state/settings/schedules snapshot |
| `mirror_get_info_changes` | `OnAir1.MirrorSave.GetInfoChanges` | MirrorSave | `` | N+1: request only changed state since previous query |
| `mirror_set_idle` | `OnAir1.MirrorSave.SetIdle` | MirrorSave | `` | N+1: switch reserve onAir to idle mode |
| `mirror_restore` | `OnAir1.MirrorSave.Restore` | MirrorSave | `` | N+1: restore state from mirrored data |


