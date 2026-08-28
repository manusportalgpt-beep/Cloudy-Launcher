/*
 * Cloudy Launcher — mock state layer for browser preview.
 *
 * The native Qt host exposes a QWebChannel "bridge" object with data properties
 * (instanceData, accountData, firstRun, …) and methods (launchInstance,
 * openModInstaller, addOfflineAccount, …).  In the browser preview there is no
 * native host, so this module creates an equivalent bridge backed by
 * localStorage.  It seeds a small library and account so the launcher is alive
 * on first open, and implements every method the web shell calls so all
 * buttons work.
 */
(() => {
  "use strict";

  const STORAGE_KEY = "cloudy-mock-state-v1";

  const defaultState = {
    firstRun: false,
    instances: [
      { id: "inst-vanilla", name: "Cloudy Vanilla", version: "1.21.4", loader: "vanilla", running: false, managed: false, mods: [] },
      { id: "inst-fabric", name: "Fabric Sandbox", version: "1.21.4", loader: "fabric", running: false, managed: false, mods: [
        { id: "mod-sodium", name: "Sodium", icon: "", version: "0.6.13" },
        { id: "mod-lithium", name: "Lithium", icon: "", version: "0.14.7" },
      ] },
      { id: "inst-forge", name: "Forge Adventures", version: "1.20.1", loader: "forge", running: false, managed: true, managedType: "CurseForge", managedVersion: "1.20.1", mods: [
        { id: "mod-jei", name: "Just Enough Items", icon: "", version: "16.0.3.106" },
      ] },
    ],
    accounts: [
      { name: "CloudyPlayer", type: "offline", active: true, uuid: "off-cloudyplayer-0000", face: null },
    ],
    globalMaxMemory: 4096,
    soundsEnabled: true,
    selectedInstanceId: "",
    curseForgeKey: "",
  };

  function load() {
    try {
      const raw = localStorage.getItem(STORAGE_KEY);
      if (raw) return JSON.parse(raw);
    } catch (_) {}
    return JSON.parse(JSON.stringify(defaultState));
  }

  function save() {
    try { localStorage.setItem(STORAGE_KEY, JSON.stringify(state)); } catch (_) {}
  }

  const state = load();
  const listeners = [];
  function emit() { listeners.forEach((fn) => fn()); }

  function genId() {
    return "inst-" + Date.now().toString(36) + Math.random().toString(36).slice(2, 5);
  }

  window.CloudyMockBridge = {
    stateChanged: { connect(fn) { listeners.push(fn); } },

    /* --- data properties read by state() in app.js --- */
    get instanceData() { return state.instances; },
    get accountData() { return state.accounts; },
    get firstRun() { return state.firstRun; },
    get hasActiveAccount() { return state.accounts.some((a) => a.active); },
    get activeAccount() { return state.accounts.find((a) => a.active)?.name || ""; },
    get globalMaxMemory() { return state.globalMaxMemory; },
    get soundsEnabled() { return state.soundsEnabled; },
    get selectedInstanceId() { return state.selectedInstanceId; },
    set selectedInstanceId(v) { state.selectedInstanceId = v; save(); },
    get storageData() {
      return {
        applicationBytes: 152000000,
        dataBytes: state.instances.length * 380000000 + 24000000,
        diskAvailableBytes: 58000000000,
      };
    },
    get paletteData() { return {}; },
    get language() {
      try { return localStorage.getItem("cloudy-language") === "ru" ? "ru_RU" : "en_US"; } catch (_) { return "en_US"; }
    },

    /* --- methods called via callNative() --- */
    launchInstance(id) {
      const inst = state.instances.find((i) => i.id === id);
      if (inst) { inst.running = true; save(); emit(); }
    },
    stopInstance(id) {
      const inst = state.instances.find((i) => i.id === id);
      if (inst) { inst.running = false; save(); emit(); }
    },
    selectInstance(id) { state.selectedInstanceId = id; save(); },
    refreshInstances() { emit(); },
    completeFirstRun() { state.firstRun = false; save(); emit(); },

    addOfflineAccount(name) {
      state.accounts.forEach((a) => (a.active = false));
      state.accounts.push({ name, type: "offline", active: true, uuid: "off-" + name.toLowerCase().replace(/[^a-z0-9]/g, "") + "-0000", face: null });
      save(); emit();
    },

    setGlobalMemory(v) { state.globalMaxMemory = Number(v) || 4096; save(); },
    setSoundsEnabled(v) { state.soundsEnabled = Boolean(v); save(); emit(); },
    setWeatherTheme() {}, setSnowVariant() {}, setLanguage() {},
    setCurseForgeApiKey(k) { state.curseForgeKey = k; save(); },

    /* no-ops in browser mode (would open native dialogs/pages) */
    openAccounts() {}, openSettings() {}, openNewInstance() {},
    openSkinStudio() {}, openMods() {}, openInstancePage() {}, openModInstaller() {},
    minimizeWindow() {}, toggleMaximizeWindow() {}, closeWindow() {}, beginWindowDrag() {},

    /* --- instance creation (used by the functional builder form) --- */
    createInstance(name, version, loader) {
      const inst = { id: genId(), name: name || "New Instance", version: version || "1.21.4", loader: loader || "vanilla", running: false, managed: false, mods: [] };
      state.instances.push(inst);
      save();
      return inst;
    },
    deleteInstance(id) {
      state.instances = state.instances.filter((i) => i.id !== id);
      save(); emit();
    },

    /* --- mod management (no emit — UI updates in place to preserve search) --- */
    addMod(instanceId, mod) {
      const inst = state.instances.find((i) => i.id === instanceId);
      if (inst && !inst.mods.some((m) => m.id === mod.id)) {
        inst.mods.push(mod);
        save();
      }
    },
    removeMod(instanceId, modId) {
      const inst = state.instances.find((i) => i.id === instanceId);
      if (inst) { inst.mods = (inst.mods || []).filter((m) => m.id !== modId); save(); }
    },
  };
})();
