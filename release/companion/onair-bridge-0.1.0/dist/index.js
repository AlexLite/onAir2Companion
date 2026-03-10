import { combineRgb, InstanceBase, InstanceStatus, runEntrypoint, TCPHelper, } from '@companion-module/base';
const COMMAND_TEMPLATES = [
    { id: 'player_time', command: 'Player.Time', category: 'Player', args: '?', description: 'Get current player time' },
    { id: 'player_block', command: 'Player.Block', category: 'Player', args: '?', description: 'Get current block playback info' },
    { id: 'player_block_info', command: 'Player.BlockInfo', category: 'Player', args: '0 0 ?', description: 'Get detailed block info by schedule/block number' },
    { id: 'player_video', command: 'Player.Video', category: 'Player', args: '1', description: 'Switch to input video line' },
    { id: 'player_switch_video', command: 'Player.SwitchVideo', category: 'Player', args: '1 2' },
    { id: 'player_default', command: 'Player.Default', category: 'Player', args: '1' },
    { id: 'player_title', command: 'Player.Title', category: 'Player', args: '?' },
    { id: 'player_title_button', command: 'Player.TitleButton', category: 'Player', args: '?' },
    { id: 'player_title_object', command: 'Player.TitleObject', category: 'Player', args: '?' },
    { id: 'player_title_object_list', command: 'Player.TitleObjectList', category: 'Player', args: '?' },
    { id: 'player_set_title_button', command: 'Player.SetTitleButton', category: 'Player', args: '0 file.xml' },
    { id: 'player_set_title_object', command: 'Player.SetTitleObject', category: 'Player', args: '0 file.xml' },
    { id: 'player_set_title_objects', command: 'Player.SetTitleObjects', category: 'Player', args: 'list.xml' },
    { id: 'player_set_title_object_task', command: 'Player.SetTitleObjectTask', category: 'Player', args: '0 task.xml' },
    { id: 'player_reload_title_button', command: 'Player.ReloadTitleButton', category: 'Player', args: '0' },
    { id: 'player_clear_title_button', command: 'Player.ClearTitleButton', category: 'Player', args: '0' },
    { id: 'player_abort_title_button', command: 'Player.AbortTitleButton', category: 'Player', args: '0' },
    { id: 'player_abort_title_object', command: 'Player.AbortTitleObject', category: 'Player', args: '0' },
    { id: 'player_abort_title_objects', command: 'Player.AbortTitleObjects', category: 'Player', args: '' },
    { id: 'player_prepare_video', command: 'Player.PrepareVideo', category: 'Player', args: '1 00:00:00.00' },
    { id: 'player_pause', command: 'Player.Pause', category: 'Player', args: '1', description: 'Pause or resume player' },
    { id: 'player_mute', command: 'Player.Mute', category: 'Player', args: '1' },
    { id: 'player_set_mute', command: 'Player.SetMute', category: 'Player', args: '0/1' },
    { id: 'player_volume', command: 'Player.Volume', category: 'Player', args: '?' },
    { id: 'player_set_volume', command: 'Player.SetVolume', category: 'Player', args: '100' },
    { id: 'player_aux', command: 'Player.Aux', category: 'Player', args: '?' },
    { id: 'player_set_aux', command: 'Player.SetAux', category: 'Player', args: '1' },
    { id: 'player_turn_aux', command: 'Player.TurnAux', category: 'Player', args: '0/1' },
    { id: 'player_titling', command: 'Player.Titling', category: 'Player', args: '?' },
    { id: 'player_set_titling', command: 'Player.SetTitling', category: 'Player', args: '0/1' },
    { id: 'player_turn_titling', command: 'Player.TurnTitling', category: 'Player', args: '' },
    { id: 'player_logotip', command: 'Player.Logotip', category: 'Player', args: '?' },
    { id: 'player_set_logotip', command: 'Player.SetLogotip', category: 'Player', args: '0/1' },
    { id: 'player_turn_logotip', command: 'Player.TurnLogotip', category: 'Player', args: '' },
    { id: 'player_fore_fade', command: 'Player.ForeFade', category: 'Player', args: '?' },
    { id: 'player_set_fore_fade', command: 'Player.SetForeFade', category: 'Player', args: '10' },
    { id: 'player_back_fade', command: 'Player.BackFade', category: 'Player', args: '?' },
    { id: 'player_set_back_fade', command: 'Player.SetBackFade', category: 'Player', args: '10' },
    { id: 'player_sound', command: 'Player.Sound', category: 'Player', args: '?' },
    { id: 'player_sound_file_stop', command: 'Player.SoundFileStop', category: 'Player', args: '' },
    { id: 'player_audio_bound', command: 'Player.AudioBound', category: 'Player', args: '?' },
    { id: 'player_set_audio_bound', command: 'Player.SetAudioBound', category: 'Player', args: '0/1' },
    { id: 'player_peak', command: 'Player.Peak', category: 'Player', args: '?' },
    { id: 'player_line_enabled', command: 'Player.LineEnabled', category: 'Player', args: '0 ?' },
    { id: 'player_set_wait_pause', command: 'Player.SetWaitPause', category: 'Player', args: '0/1' },
    { id: 'player_set_wait_sound', command: 'Player.SetWaitSound', category: 'Player', args: '0/1' },
    { id: 'player_set_wait_video', command: 'Player.SetWaitVideo', category: 'Player', args: '0/1' },
    { id: 'player_set_wait_title', command: 'Player.SetWaitTitle', category: 'Player', args: '0/1' },
    { id: 'player_fore_title_stop', command: 'Player.ForeTitleStop', category: 'Player', args: '' },
    { id: 'player_pure_video', command: 'Player.PureVideo', category: 'Player', args: '0/1' },
    { id: 'shedule_start', command: 'Shedule.Start', category: 'Shedule', args: '', description: 'Start current schedule' },
    { id: 'shedule_pause', command: 'Shedule.Pause', category: 'Shedule', args: '0/1' },
    { id: 'shedule_set_pause', command: 'Shedule.SetPause', category: 'Shedule', args: '0/1' },
    { id: 'shedule_break', command: 'Shedule.Break', category: 'Shedule', args: '' },
    { id: 'shedule_skip_block', command: 'Shedule.SkipBlock', category: 'Shedule', args: '0' },
    { id: 'shedule_active', command: 'Shedule.ActiveShedule', category: 'Shedule', args: '0 ?' },
    { id: 'shedule_run_position', command: 'Shedule.RunPosition', category: 'Shedule', args: '0 ?' },
    { id: 'shedule_set_run_position', command: 'Shedule.SetRunPosition', category: 'Shedule', args: '0 1' },
    { id: 'shedule_show_first', command: 'Shedule.ShowFirst', category: 'Shedule', args: '0' },
    { id: 'shedule_length', command: 'Shedule.Length', category: 'Shedule', args: '0 ?' },
    { id: 'shedule_item', command: 'Shedule.Item', category: 'Shedule', args: '0 1 ?' },
    { id: 'shedule_item_ex', command: 'Shedule.ItemEx', category: 'Shedule', args: '0 1 ?' },
    { id: 'shedule_set_item', command: 'Shedule.SetItem', category: 'Shedule', args: '0 1 command...' },
    { id: 'shedule_add_item', command: 'Shedule.AddItem', category: 'Shedule', args: '0 command...' },
    { id: 'shedule_delete_item', command: 'Shedule.DeleteItem', category: 'Shedule', args: '0 1' },
    { id: 'shedule_delete_item_above', command: 'Shedule.DeleteItemAbove', category: 'Shedule', args: '0 1' },
    { id: 'shedule_erase_list', command: 'Shedule.EraseList', category: 'Shedule', args: '0' },
    { id: 'shedule_save_to_file', command: 'Shedule.SaveToFile', category: 'Shedule', args: '0 D:/example.air' },
    { id: 'shedule_read_from_file', command: 'Shedule.ReadFromFile', category: 'Shedule', args: '0 D:/example.air' },
    { id: 'shedule_insert_from_file', command: 'Shedule.InsertFromFile', category: 'Shedule', args: '0 1 D:/example.air' },
    { id: 'shedule_check_state', command: 'Shedule.CheckState', category: 'Shedule', args: '', description: 'Check command states in schedule' },
    { id: 'shedule_time_shift', command: 'Shedule.TimeShift', category: 'Shedule', args: '0 1 -1000' },
    { id: 'shedule_set_run_shedule', command: 'Shedule.SetRunShedule', category: 'Shedule', args: '0/1' },
    { id: 'shedule_turn_run_shedule', command: 'Shedule.TurnRunShedule', category: 'Shedule', args: '' },
    { id: 'shedule_insert_mode', command: 'Shedule.InsertMode', category: 'Shedule', args: '?' },
    { id: 'shedule_set_insert_mode', command: 'Shedule.SetInsertMode', category: 'Shedule', args: '0/1' },
    { id: 'shedule_edit_position', command: 'Shedule.EditPosition', category: 'Shedule', args: '0 ?' },
    { id: 'shedule_set_edit_position', command: 'Shedule.SetEditPosition', category: 'Shedule', args: '0 1' },
    { id: 'shedule_set_item_name', command: 'Shedule.SetItemName', category: 'Shedule', args: '0 1 Name' },
    { id: 'shedule_set_item_title', command: 'Shedule.SetItemTitle', category: 'Shedule', args: '0 1 Title' },
    { id: 'shedule_set_item_duration', command: 'Shedule.SetItemDuration', category: 'Shedule', args: '0 1 00:00:05.00' },
    { id: 'shedule_set_item_fade', command: 'Shedule.SetItemFade', category: 'Shedule', args: '0 1 10' },
    { id: 'shedule_set_item_wait_type', command: 'Shedule.SetItemWaitType', category: 'Shedule', args: '0 1 follow' },
    { id: 'shedule_set_item_wait_start', command: 'Shedule.SetItemWaitStart', category: 'Shedule', args: '0 1 12:00:00.00' },
    { id: 'shedule_set_item_wait_duration', command: 'Shedule.SetItemWaitDuration', category: 'Shedule', args: '0 1 5.00' },
    { id: 'shedule_set_item_wait_decrease', command: 'Shedule.SetItemWaitDecrease', category: 'Shedule', args: '0 1 0/1' },
    { id: 'shedule_set_item_logo', command: 'Shedule.SetItemLogo', category: 'Shedule', args: '0 1 0/1' },
    { id: 'shedule_set_item_inout', command: 'Shedule.SetItemInOut', category: 'Shedule', args: '0 1 in out' },
    { id: 'shedule_lock_run', command: 'Shedule.LockRun', category: 'Shedule', args: '?' },
    { id: 'shedule_set_lock_run', command: 'Shedule.SetLockRun', category: 'Shedule', args: '0/1' },
    { id: 'shedule_get_next_error_line', command: 'Shedule.GetNextErrorLine', category: 'Shedule', args: '0 ?' },
    { id: 'folder_info', command: 'Folder.Info', category: 'Folder', args: 'D:/', description: 'Get folder listing/info' },
    { id: 'folder_file_info', command: 'Folder.FileInfo', category: 'Folder', args: 'D:/file.avi', description: 'Get file info' },
    { id: 'folder_refresh', command: 'Folder.Refresh', category: 'Folder', args: 'D:/', description: 'Refresh folder metadata cache' },
    { id: 'folder_summary', command: 'Folder.Summary', category: 'Folder', args: 'D:/', description: 'Get folder summary' },
    { id: 'setting_name', command: 'Setting.Name', category: 'Setting', args: '?', description: 'Get onAir instance name' },
    { id: 'loading_break', command: 'Loading.Break', category: 'Loading', args: '', description: 'Interrupt loading process' },
    { id: 'mirror_get_info_all', command: 'OnAir1.MirrorSave.GetInfoAll', category: 'MirrorSave', args: '', description: 'N+1: request full state/settings/schedules snapshot' },
    { id: 'mirror_get_info_changes', command: 'OnAir1.MirrorSave.GetInfoChanges', category: 'MirrorSave', args: '', description: 'N+1: request only changed state since previous query' },
    { id: 'mirror_set_idle', command: 'OnAir1.MirrorSave.SetIdle', category: 'MirrorSave', args: '', description: 'N+1: switch reserve onAir to idle mode' },
    { id: 'mirror_restore', command: 'OnAir1.MirrorSave.Restore', category: 'MirrorSave', args: '', description: 'N+1: restore state from mirrored data' },
];
class OnAirBridgeInstance extends InstanceBase {
    config = {};
    tcp;
    lineBuffer = '';
    requestId = 1;
    bridgeState = 'disconnected';
    lastMessage = '';
    lastBody = '';
    lastSubject = '';
    lastFromMachine = '';
    lastFromQueue = '';
    lastResponse = '';
    lastError = '';
    async init(config) {
        this.initVariables();
        this.setActionDefinitions(this.getActions());
        this.setFeedbackDefinitions(this.getFeedbacks());
        this.setPresetDefinitions(this.getPresets());
        await this.configUpdated(config);
    }
    async configUpdated(config) {
        this.config = config;
        this.setPresetDefinitions(this.getPresets());
        this.connectTcp();
    }
    async destroy() {
        this.disconnectTcp();
    }
    getConfigFields() {
        return [
            { type: 'textinput', id: 'host', label: 'Bridge Host', width: 8, default: '192.168.11.71' },
            { type: 'number', id: 'port', label: 'Bridge Port', width: 4, default: 27015, min: 1, max: 65535 },
            { type: 'number', id: 'reconnectMs', label: 'Reconnect (ms)', width: 4, default: 1000, min: 100, max: 30000 },
            { type: 'textinput', id: 'defaultMachine', label: 'Default machine', width: 4, default: 'SERVER1' },
            { type: 'textinput', id: 'defaultQueue', label: 'Default queue', width: 4, default: 'FDOnAir1' },
            { type: 'textinput', id: 'defaultSubject', label: 'Default subject', width: 4, default: 'OnAir1.Mirror' },
            { type: 'textinput', id: 'cmdPlay', label: 'Preset PLAY command', width: 3, default: 'Player.Play' },
            { type: 'textinput', id: 'cmdStop', label: 'Preset STOP command', width: 3, default: 'Player.Stop' },
            { type: 'textinput', id: 'cmdNext', label: 'Preset NEXT command', width: 3, default: 'Player.Next' },
            { type: 'textinput', id: 'cmdTake', label: 'Preset TAKE command', width: 3, default: 'Player.Take' },
        ];
    }
    initVariables() {
        const vars = [
            { variableId: 'bridge_state', name: 'Bridge state' },
            { variableId: 'last_message', name: 'Last raw message' },
            { variableId: 'last_body', name: 'Last onAir body' },
            { variableId: 'last_subject', name: 'Last onAir subject' },
            { variableId: 'last_from_machine', name: 'Last from machine' },
            { variableId: 'last_from_queue', name: 'Last from queue' },
            { variableId: 'last_response', name: 'Last response' },
            { variableId: 'last_error', name: 'Last error' },
        ];
        this.setVariableDefinitions(vars);
        this.updateVariables();
    }
    updateVariables() {
        this.setVariableValues({
            bridge_state: this.bridgeState,
            last_message: this.lastMessage,
            last_body: this.lastBody,
            last_subject: this.lastSubject,
            last_from_machine: this.lastFromMachine,
            last_from_queue: this.lastFromQueue,
            last_response: this.lastResponse,
            last_error: this.lastError,
        });
    }
    getActions() {
        return {
            send_command: {
                name: 'Send command',
                options: [
                    { type: 'textinput', id: 'command', label: 'Command', default: 'Shedule.Start', useVariables: true },
                    { type: 'textinput', id: 'machine', label: 'Machine (optional)', default: '', useVariables: true },
                    { type: 'textinput', id: 'queue', label: 'Queue (optional)', default: '', useVariables: true },
                    { type: 'textinput', id: 'subject', label: 'Subject (optional)', default: '', useVariables: true },
                ],
                callback: async (action, context) => {
                    const command = await context.parseVariablesInString(String(action.options.command ?? ''));
                    const machineRaw = await context.parseVariablesInString(String(action.options.machine ?? ''));
                    const queueRaw = await context.parseVariablesInString(String(action.options.queue ?? ''));
                    const subjectRaw = await context.parseVariablesInString(String(action.options.subject ?? ''));
                    if (!command.trim())
                        return;
                    await this.sendJson({
                        id: String(this.requestId++),
                        action: 'send',
                        command,
                        to_machine: machineRaw.trim() || this.config.defaultMachine || undefined,
                        to_queue: queueRaw.trim() || this.config.defaultQueue || undefined,
                        subject: subjectRaw.trim() || this.config.defaultSubject || undefined,
                    });
                },
            },
            send_template: {
                name: 'Send template command (FDOnAir)',
                options: [
                    {
                        type: 'dropdown',
                        id: 'template',
                        label: 'Template',
                        default: COMMAND_TEMPLATES[0]?.id || 'player_time',
                        choices: COMMAND_TEMPLATES.map((c) => ({
                            id: c.id,
                            label: `${c.command} ${c.args}`.trim() + (c.description ? ` — ${c.description}` : ''),
                        })),
                    },
                    { type: 'textinput', id: 'args', label: 'Args override', default: '', useVariables: true },
                    { type: 'textinput', id: 'machine', label: 'Machine (optional)', default: '', useVariables: true },
                    { type: 'textinput', id: 'queue', label: 'Queue (optional)', default: '', useVariables: true },
                    { type: 'textinput', id: 'subject', label: 'Subject (optional)', default: '', useVariables: true },
                ],
                callback: async (action, context) => {
                    const templateId = String(action.options.template ?? '');
                    const argsRaw = await context.parseVariablesInString(String(action.options.args ?? ''));
                    const machineRaw = await context.parseVariablesInString(String(action.options.machine ?? ''));
                    const queueRaw = await context.parseVariablesInString(String(action.options.queue ?? ''));
                    const subjectRaw = await context.parseVariablesInString(String(action.options.subject ?? ''));
                    const tpl = COMMAND_TEMPLATES.find((c) => c.id === templateId);
                    if (!tpl)
                        return;
                    await this.sendJson({
                        id: String(this.requestId++),
                        action: 'send',
                        command: this.composeTemplateCommand(tpl, argsRaw),
                        to_machine: machineRaw.trim() || this.config.defaultMachine || undefined,
                        to_queue: queueRaw.trim() || this.config.defaultQueue || undefined,
                        subject: subjectRaw.trim() || this.config.defaultSubject || undefined,
                    });
                },
            },
            send_json: {
                name: 'Send JSON line',
                options: [{ type: 'textinput', id: 'json', label: 'JSON', default: '{"action":"health"}', useVariables: true }],
                callback: async (action, context) => {
                    const json = await context.parseVariablesInString(String(action.options.json ?? ''));
                    await this.sendRaw(json);
                },
            },
            health: {
                name: 'Health check',
                options: [],
                callback: async () => {
                    await this.sendJson({ id: String(this.requestId++), action: 'health' });
                },
            },
        };
    }
    getFeedbacks() {
        return {
            bridge_connected: {
                type: 'boolean',
                name: 'Bridge connected',
                defaultStyle: { bgcolor: combineRgb(0, 128, 0), color: combineRgb(255, 255, 255) },
                options: [],
                callback: () => this.bridgeState === 'connected',
            },
            last_body_contains: {
                type: 'boolean',
                name: 'Last body contains',
                defaultStyle: { bgcolor: combineRgb(20, 100, 170), color: combineRgb(255, 255, 255) },
                options: [{ type: 'textinput', id: 'needle', label: 'Text', default: 'Test.Bridge.Ping', useVariables: true }],
                callback: (feedback) => {
                    const needle = String(feedback.options.needle ?? '').trim();
                    if (!needle)
                        return false;
                    return this.lastBody.includes(needle);
                },
            },
            message_match: {
                type: 'boolean',
                name: 'Message match (contains/equals/regex)',
                defaultStyle: { bgcolor: combineRgb(128, 90, 0), color: combineRgb(255, 255, 255) },
                options: [
                    { type: 'dropdown', id: 'mode', label: 'Mode', default: 'contains', choices: [
                            { id: 'contains', label: 'Contains' },
                            { id: 'equals', label: 'Equals' },
                            { id: 'regex', label: 'Regex' },
                        ] },
                    { type: 'textinput', id: 'pattern', label: 'Pattern', default: 'Test.Bridge.Ping', useVariables: true },
                    { type: 'textinput', id: 'subject', label: 'Subject filter (optional)', default: '', useVariables: true },
                    { type: 'textinput', id: 'from_queue', label: 'From queue filter (optional)', default: '', useVariables: true },
                ],
                callback: (feedback) => {
                    const mode = String(feedback.options.mode ?? 'contains');
                    const pattern = String(feedback.options.pattern ?? '');
                    const subjectFilter = String(feedback.options.subject ?? '').trim();
                    const queueFilter = String(feedback.options.from_queue ?? '').trim();
                    if (!pattern.trim())
                        return false;
                    if (subjectFilter && this.lastSubject !== subjectFilter)
                        return false;
                    if (queueFilter && this.lastFromQueue !== queueFilter)
                        return false;
                    return this.matchBody(mode, pattern);
                },
            },
        };
    }
    getPresets() {
        const playCmd = String(this.config.cmdPlay || 'Player.Play');
        const stopCmd = String(this.config.cmdStop || 'Player.Stop');
        const nextCmd = String(this.config.cmdNext || 'Player.Next');
        const takeCmd = String(this.config.cmdTake || 'Player.Take');
        return {
            ping_test: {
                type: 'button',
                category: 'onAir Bridge',
                name: 'Send Test.Bridge.Ping',
                style: { text: 'PING', size: '14', color: combineRgb(255, 255, 255), bgcolor: combineRgb(0, 0, 0) },
                steps: [{ down: [{ actionId: 'send_command', options: { command: 'Test.Bridge.Ping', machine: '', queue: '', subject: '' } }], up: [] }],
                feedbacks: [{ feedbackId: 'last_body_contains', options: { needle: 'Test.Bridge.Ping' }, style: { bgcolor: combineRgb(0, 128, 0), color: combineRgb(255, 255, 255) } }],
            },
            health: {
                type: 'button',
                category: 'onAir Bridge',
                name: 'Health',
                style: { text: 'HEALTH', size: '14', color: combineRgb(255, 255, 255), bgcolor: combineRgb(0, 0, 0) },
                steps: [{ down: [{ actionId: 'health', options: {} }], up: [] }],
                feedbacks: [{ feedbackId: 'bridge_connected', options: {}, style: { bgcolor: combineRgb(0, 128, 0), color: combineRgb(255, 255, 255) } }],
            },
            play: this.makeCommandPreset('PLAY', 'Play', playCmd, combineRgb(0, 90, 0)),
            stop: this.makeCommandPreset('STOP', 'Stop', stopCmd, combineRgb(120, 0, 0)),
            next: this.makeCommandPreset('NEXT', 'Next', nextCmd, combineRgb(0, 0, 120)),
            take: this.makeCommandPreset('TAKE', 'Take', takeCmd, combineRgb(90, 60, 0)),
            ...this.makeTemplatePresets(),
        };
    }
    makeCommandPreset(buttonText, name, command, activeColor) {
        return {
            type: 'button',
            category: 'onAir Control',
            name,
            style: { text: buttonText, size: '14', color: combineRgb(255, 255, 255), bgcolor: combineRgb(0, 0, 0) },
            steps: [{ down: [{ actionId: 'send_command', options: { command, machine: '', queue: '', subject: '' } }], up: [] }],
            feedbacks: [{ feedbackId: 'message_match', options: { mode: 'contains', pattern: command, subject: '', from_queue: '' }, style: { bgcolor: activeColor, color: combineRgb(255, 255, 255) } }],
        };
    }
    makeTemplatePresets() {
        const out = {};
        for (const tpl of COMMAND_TEMPLATES) {
            const id = `tpl_${tpl.id}`;
            const short = tpl.command.replace(/^([A-Za-z]+)\./, '');
            const text = short.length > 10 ? short.slice(0, 10) : short;
            const category = `FDOnAir ${tpl.category}`;
            const command = this.composeTemplateCommand(tpl, '');
            out[id] = {
                type: 'button',
                category,
                name: `${tpl.command} ${tpl.args}`.trim() + (tpl.description ? ` — ${tpl.description}` : ''),
                style: { text, size: '14', color: combineRgb(255, 255, 255), bgcolor: combineRgb(0, 0, 0) },
                steps: [{ down: [{ actionId: 'send_command', options: { command, machine: '', queue: '', subject: '' } }], up: [] }],
                feedbacks: [],
            };
        }
        return out;
    }
    connectTcp() {
        this.disconnectTcp();
        const host = String(this.config.host || '').trim();
        const port = Number(this.config.port || 0);
        if (!host || !port) {
            this.bridgeState = 'error';
            this.lastError = 'Invalid host or port in config';
            this.updateVariables();
            this.updateStatus(InstanceStatus.BadConfig, this.lastError);
            return;
        }
        this.bridgeState = 'connecting';
        this.updateVariables();
        this.updateStatus(InstanceStatus.Connecting);
        this.tcp = new TCPHelper(host, port, { reconnect: true, reconnect_interval: Number(this.config.reconnectMs || 1000) });
        this.tcp.on('connect', () => {
            this.bridgeState = 'connected';
            this.lastError = '';
            this.updateVariables();
            this.updateStatus(InstanceStatus.Ok);
            this.checkFeedbacks('bridge_connected', 'last_body_contains', 'message_match');
            this.log('info', `Connected to ${host}:${port}`);
        });
        this.tcp.on('error', (err) => {
            this.bridgeState = 'error';
            this.lastError = err.message;
            this.updateVariables();
            this.updateStatus(InstanceStatus.UnknownError, err.message);
            this.checkFeedbacks('bridge_connected');
        });
        this.tcp.on('end', () => {
            this.bridgeState = 'disconnected';
            this.updateVariables();
            this.updateStatus(InstanceStatus.Disconnected);
            this.checkFeedbacks('bridge_connected');
        });
        this.tcp.on('data', (msg) => {
            this.consumeData(msg.toString('utf8'));
        });
        this.tcp.on('status_change', (status, message) => {
            if (status === InstanceStatus.Connecting)
                this.bridgeState = 'connecting';
            if (status === InstanceStatus.Disconnected)
                this.bridgeState = 'disconnected';
            if (status === InstanceStatus.Ok)
                this.bridgeState = 'connected';
            if (status === InstanceStatus.UnknownError)
                this.bridgeState = 'error';
            if (message)
                this.lastError = message;
            this.updateVariables();
            this.updateStatus(status, message);
            this.checkFeedbacks('bridge_connected');
        });
    }
    disconnectTcp() {
        if (this.tcp) {
            this.tcp.destroy();
            this.tcp.removeAllListeners();
            this.tcp = undefined;
        }
        this.lineBuffer = '';
    }
    consumeData(chunk) {
        this.lineBuffer += chunk;
        for (;;) {
            const idx = this.lineBuffer.indexOf('\n');
            if (idx < 0)
                break;
            const rawLine = this.lineBuffer.slice(0, idx).replace(/\r$/, '');
            this.lineBuffer = this.lineBuffer.slice(idx + 1);
            if (!rawLine.trim())
                continue;
            this.handleLine(rawLine);
        }
    }
    handleLine(line) {
        this.lastMessage = line;
        let parsed;
        try {
            parsed = JSON.parse(line);
        }
        catch {
            this.updateVariables();
            return;
        }
        if (parsed.body !== undefined)
            this.lastBody = String(parsed.body);
        if (parsed.subject !== undefined)
            this.lastSubject = String(parsed.subject);
        if (parsed.from_machine !== undefined)
            this.lastFromMachine = String(parsed.from_machine);
        if (parsed.from_queue !== undefined)
            this.lastFromQueue = String(parsed.from_queue);
        if (parsed.type === 'response')
            this.lastResponse = line;
        if (parsed.error)
            this.lastError = String(parsed.error);
        this.updateVariables();
        this.checkFeedbacks('bridge_connected', 'last_body_contains', 'message_match');
    }
    matchBody(mode, pattern) {
        if (mode === 'equals')
            return this.lastBody === pattern;
        if (mode === 'regex') {
            try {
                return new RegExp(pattern).test(this.lastBody);
            }
            catch {
                return false;
            }
        }
        return this.lastBody.includes(pattern);
    }
    composeTemplateCommand(template, argsOverride) {
        const args = argsOverride.trim() || template.args;
        if (!args || args === '?')
            return template.command;
        return `${template.command} ${args}`.trim();
    }
    async sendJson(payload) {
        await this.sendRaw(JSON.stringify(payload));
    }
    async sendRaw(line) {
        if (!this.tcp) {
            this.log('error', 'TCP is not initialized');
            return;
        }
        const text = line.endsWith('\n') ? line : `${line}\n`;
        try {
            await this.tcp.send(text);
        }
        catch (err) {
            const msg = err instanceof Error ? err.message : String(err);
            this.lastError = msg;
            this.updateVariables();
            this.updateStatus(InstanceStatus.UnknownError, msg);
            this.log('error', `Send failed: ${msg}`);
        }
    }
}
runEntrypoint(OnAirBridgeInstance, []);
