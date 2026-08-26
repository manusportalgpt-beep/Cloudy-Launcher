(() => {
  "use strict";

  const workspace = document.getElementById("workspace");
  const crumb = document.getElementById("crumb");
  const profileName = document.getElementById("profile-name");
  const profileAvatar = document.getElementById("profile-avatar");
  const profileAvatarLetter = document.getElementById("profile-avatar-letter");
  const profileAvatarImage = document.getElementById("profile-avatar-image");
  const topbarRunningText = document.getElementById("topbar-running-text");
  const navButtons = [...document.querySelectorAll(".rail-button[data-route]")];
  let bridge = null;
  let route = "home";

  const weatherThemes = {
    cloudy: { label: "Cloudy", description: "Soft clouds, sun breaks, leafy trees and distant mountains" },
    rain: { label: "Rain", description: "Fine rain, pale clouds and a few quiet puddles" },
    storm: { label: "Storm", description: "Heavy rain, dark clouds, fog, wind and large puddles" },
    sunny: { label: "Sunny", description: "A bright summer sky with warm light and white clouds" },
    night: { label: "Night", description: "Stars, constellations and a slow midnight sky" },
    snow: { label: "Snow", description: "White fog, falling snow, bare trees and snowy mountains" },
  };

  const readWeatherTheme = () => {
    try {
      const saved = localStorage.getItem("cloudy-weather-theme");
      return weatherThemes[saved] ? saved : "cloudy";
    } catch (_) {
      return "cloudy";
    }
  };

  const readSnowVariant = () => {
    try {
      const saved = localStorage.getItem("cloudy-snow-variant");
      return saved === "dark" ? "dark" : "light";
    } catch (_) {
      return "light";
    }
  };

  const applyWeatherTheme = (theme = readWeatherTheme()) => {
    const safeTheme = weatherThemes[theme] ? theme : "cloudy";
    document.documentElement.dataset.weatherTheme = safeTheme;
    document.documentElement.dataset.snowVariant = readSnowVariant();
    return safeTheme;
  };

  const chooseWeatherTheme = (theme) => {
    const safeTheme = applyWeatherTheme(theme);
    try { localStorage.setItem("cloudy-weather-theme", safeTheme); } catch (_) {}
    render();
  };

  const escapeHtml = (value) => String(value ?? "")
    .replaceAll("&", "&amp;")
    .replaceAll("<", "&lt;")
    .replaceAll(">", "&gt;")
    .replaceAll('"', "&quot;")
    .replaceAll("'", "&#039;");

  const callNative = (method, ...args) => {
    if (bridge && typeof bridge[method] === "function") {
      bridge[method](...args);
    }
  };

  const state = () => {
    const values = bridge?.instanceData;
    return {
      instances: Array.isArray(values) ? values : [],
      accounts: Array.isArray(bridge?.accountData) ? bridge.accountData : [],
      account: bridge?.activeAccount || "",
      hasAccount: Boolean(bridge?.hasActiveAccount),
      palette: bridge?.paletteData || {},
      weatherTheme: readWeatherTheme(),
      snowVariant: readSnowVariant(),
    };
  };

  const applyPalette = (palette) => {
    const root = document.documentElement;
    const map = { "--text": "text", "--white": "highlightedText" };
    Object.entries(map).forEach(([cssName, paletteName]) => {
      if (palette?.[paletteName]) root.style.setProperty(cssName, palette[paletteName]);
    });
  };

  const instanceLabel = (instance) => {
    if (instance.managed && instance.managedType) {
      return `${instance.managedType} · ${instance.managedVersion || "managed pack"}`;
    }
    return instance.running ? "Running now" : "Ready to play";
  };

  const setRoute = (nextRoute) => {
    route = nextRoute;
    navButtons.forEach((button) => button.classList.toggle("is-active", button.dataset.route === route));
    const labels = { home: "Home", new: "New instance", accounts: "Accounts", settings: "Settings", skins: "Skin Studio", mods: "Mods", files: "Files" };
    crumb.textContent = labels[route] || "Home";
    render();
  };

  const shellButton = (label, action, extra = "") => `<button class="action-button ${extra}" data-action="${action}">${label}</button>`;
  const providerIconSources = {
    custom: "data:image/svg+xml;base64,CLOUDY_PROVIDER_CUSTOM",
    modrinth: "data:image/png;base64,CLOUDY_PROVIDER_MODRINTH",
    curseforge: "data:image/png;base64,CLOUDY_PROVIDER_CURSEFORGE",
    ftb: "data:image/png;base64,CLOUDY_PROVIDER_FTB",
    technic: "data:image/png;base64,CLOUDY_PROVIDER_TECHNIC",
    import: "data:image/svg+xml;base64,CLOUDY_PROVIDER_IMPORT",
  };

  const renderHome = ({ instances, account, hasAccount }) => {
    const running = instances.filter((item) => item.running).length;
    const runningText = running ? `${running} ${running === 1 ? "instance is" : "instances are"} currently running` : "No instances are currently running";
    const cards = instances.map((item) => `
      <article class="instance-card" data-instance-id="${escapeHtml(item.id)}" tabindex="0" aria-label="Open ${escapeHtml(item.name)}">
        <div class="instance-art">
          <div class="instance-placeholder">${escapeHtml((item.name || "C").slice(0, 1).toUpperCase())}</div>
        </div>
        <div class="instance-copy">
          <strong>${escapeHtml(item.name)}</strong>
          <small>${escapeHtml(instanceLabel(item))}</small>
          <span class="instance-status"><span class="status-dot"></span>${item.running ? "Playing" : "Available"}</span>
          <div class="card-actions">
            ${item.running ? shellButton("Open", "launch", "quiet") : shellButton("Launch", "launch", "primary")}
            ${shellButton("Mods", "mods", "quiet")}
            ${shellButton("Files", "files", "quiet")}
          </div>
        </div>
      </article>`).join("");

    const accountText = hasAccount ? escapeHtml(account) : "Connect an account in Accounts";
    workspace.innerHTML = `
      <section class="page library-page">
        <div class="library-top">
          <div class="copy">
            <p class="eyebrow">Cloud workspace</p>
            <h1>Greetings!</h1>
            <p class="lead">Here are your worlds and servers. Everything stays in one calm workspace.</p>
          </div>
          <div class="library-actions">
            <input class="search-box" id="instance-search" type="search" placeholder="Search instances" aria-label="Search instances">
            ${shellButton("Create instance", "new", "primary")}
          </div>
        </div>
        <div class="hero-strip">
          <div class="hero-panel">
            <div class="hero-cloud" aria-hidden="true"></div>
            <p class="eyebrow">Cloudy Launcher</p>
            <h2>Build, browse and play without leaving the flow.</h2>
            <p class="lead">Use Modrinth, CurseForge, FTB, Technic and local imports through the existing verified installers.</p>
          </div>
          <div class="side-panel"><span>Active profile</span><strong>${accountText}</strong><span>${instances.length} ${instances.length === 1 ? "instance" : "instances"}</span></div>
        </div>
        <div class="tools-row"><span class="subtle">Library</span><span class="library-status"><span class="status-dot"></span>${runningText}</span><div class="segmented"><button class="is-active" type="button">Grid</button><button type="button" data-action="refresh">Refresh</button></div></div>
        ${instances.length ? `<div class="instance-grid" id="instance-grid">${cards}</div>` : `<div class="empty-state"><button class="empty-tile" data-action="new" aria-label="Create a new instance"><span class="empty-plus"></span></button><p class="empty-caption">Your library is clear. Add a Minecraft instance to start shaping your cloud.</p></div>`}
      </section>`;
  };

  const renderNew = () => {
    const providers = [
      ["Custom", "Local version or imported pack", "custom"],
      ["Modrinth", "Modpacks, versions and projects", "modrinth"],
      ["CurseForge", "Modpacks and manifests", "curseforge"],
      ["FTB", "Official FTB packs and versions", "ftb"],
      ["Technic", "Technic platform packs", "technic"],
      ["Import", "ZIP, MRPACK or local folder", "import"],
    ];
    workspace.innerHTML = `
      <section class="page">
        <div class="page-heading"><div><p class="eyebrow">Instance builder</p><h1>Create an instance</h1><p class="lead">Choose a source, version and loader. The complete installer remains powered by Cloudy’s native task pipeline.</p></div>${shellButton("Back to library", "home", "quiet")}</div>
        <div class="builder-stepper" aria-label="Instance creation steps"><div class="builder-step is-current"><span>01</span><strong>Source</strong><small>Choose a provider</small></div><div class="builder-step"><span>02</span><strong>Version</strong><small>Release or snapshot</small></div><div class="builder-step"><span>03</span><strong>Loader</strong><small>Forge, Fabric or vanilla</small></div><div class="builder-step"><span>04</span><strong>Review</strong><small>Name and install</small></div></div><div class="flow-layout">
          <div class="feature-panel"><div class="tools-row"><h2>Choose a source</h2><span class="subtle">All providers</span></div><div class="builder-selection"><div><p class="eyebrow">Starting point</p><strong>Pick where the pack comes from</strong><span>Cloudy keeps the provider choice visible while the native builder handles the real install.</span></div><span class="selection-badge">Step 1 of 4</span></div><div class="provider-list">${providers.map(([name, detail, icon], index) => `<button class="provider-card ${index === 0 ? "is-selected" : ""}" data-provider="${icon}" data-action="open-new" aria-label="Open ${name} in the native builder"><span class="provider-logo provider-${icon}"><img src="${providerIconSources[icon]}" alt=""></span><span><strong>${name}</strong><small>${detail}</small></span></button>`).join("")}</div><div class="selection-cloud-anchor"><svg class="cloud-anchor-icon" viewBox="0 0 96 48" aria-hidden="true"><path d="M18 38h55c9 0 16-6 16-14 0-7-6-13-14-14C72 4 65 0 57 0c-8 0-15 5-17 12C30 11 22 16 22 23c-7 0-12 4-12 8 0 4 3 7 8 7Z" fill="none" stroke="currentColor" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round"/><path d="M18 38h55" fill="none" stroke="currentColor" stroke-width="2.5" stroke-linecap="round"/></svg><span>Cloudy keeps your source, version and loader choice together.</span></div><div class="native-handoff"><span>Open the complete version, loader and installer flow.</span>${shellButton("Open builder", "open-new", "primary")}</div></div>
          <aside class="side-panel"><p class="eyebrow">Build preview</p><h2>Ready for the native flow</h2><ol class="build-steps"><li><span>01</span><div><strong>Source</strong><small>Provider or local import</small></div></li><li><span>02</span><div><strong>Version</strong><small>Release, snapshot or beta</small></div></li><li><span>03</span><div><strong>Loader</strong><small>Compatible mod loader</small></div></li><li><span>04</span><div><strong>Install</strong><small>Name, icon and task progress</small></div></li></ol><div class="callout">The next step opens Cloudy’s verified native builder. Microsoft OAuth and existing account handling remain native and secure.</div></aside>
        </div>
      </section>`;
  };

  const renderMods = ({ instances }) => {
    const selected = instances[0];
    workspace.innerHTML = `
      <section class="page"><div class="page-heading"><div><p class="eyebrow">Resource workspace</p><h1>Mods & resources</h1><p class="lead">Search and install mods through the existing Modrinth, CurseForge and local resource workflows.</p></div>${shellButton("Back to library", "home", "quiet")}</div>
      <div class="hero-strip"><div class="hero-panel"><p class="eyebrow">Selected instance</p><h2>${selected ? escapeHtml(selected.name) : "No instance selected"}</h2><p class="lead">Select an instance from Library, then open its real mod page with filters, versions and install tasks.</p>${selected ? shellButton("Open native mod workspace", "open-mods", "primary") : shellButton("Choose an instance", "home", "quiet")}</div><div class="side-panel"><span>Sources</span><strong>3+</strong><span>Modrinth · CurseForge · local files</span></div></div>
      <div class="flow-layout"><div class="feature-panel"><div class="tools-row"><h2>Install source</h2><span class="subtle">Task-backed</span></div><div class="provider-list"><button class="provider-card" data-action="open-mods"><span class="provider-logo provider-modrinth"><img src="data:image/png;base64,CLOUDY_PROVIDER_MODRINTH" alt=""></span><span><strong>Modrinth</strong><small>Search projects and versions</small></span></button><button class="provider-card" data-action="open-mods"><span class="provider-logo provider-curseforge"><img src="data:image/png;base64,CLOUDY_PROVIDER_CURSEFORGE" alt=""></span><span><strong>CurseForge</strong><small>Use existing package metadata</small></span></button></div></div><aside class="side-panel"><p class="eyebrow">Provider details</p><h2>Authors stay linked</h2><p class="subtle">Open the native resource workspace for real project metadata, clickable Modrinth and CurseForge author links, versions and install tasks.</p><div class="callout">Author pages, project links and provider extras are loaded from the existing native models. Cloudy does not invent author profiles or expose provider credentials.</div></aside></div></section>`;
  };

  const renderFiles = ({ instances }) => {
    const selected = instances[0];
    workspace.innerHTML = `
      <section class="page"><div class="page-heading"><div><p class="eyebrow">Workspace files</p><h1>Files</h1><p class="lead">Open the selected instance folder with the native file model. No path is exposed to the web layer.</p></div>${shellButton("Back to library", "home", "quiet")}</div>
      <div class="empty-note"><div><h2>${selected ? `Open files for ${escapeHtml(selected.name)}` : "Select an instance first"}</h2><p class="subtle">Drag & drop and editing continue in the native, permission-aware file workflow.</p><div style="margin-top:16px">${selected ? shellButton("Open instance folder", "open-files", "primary") : shellButton("Go to library", "home", "quiet")}</div></div></div></section>`;
  };

  const renderAccounts = ({ account, hasAccount, accounts }) => {
    const accountCards = accounts.length ? accounts.map((item) => {
      const displayName = item.name || item.profileName || "Unnamed account";
      const avatar = item.face
        ? `<img class="account-face" src="${escapeHtml(item.face)}" alt="${escapeHtml(displayName)} face">`
        : `<span class="avatar-letter">${escapeHtml(displayName.slice(0, 1).toUpperCase())}</span>`;
      const uuid = item.uuid ? escapeHtml(item.uuid) : "Profile UUID unavailable";
      const type = item.type === "msa" ? "Microsoft account" : "Offline profile";
      return `<div class="account-item ${item.active ? "is-active" : ""}"><div class="account-summary"><div class="account-avatar">${avatar}</div><div class="copy"><strong>${escapeHtml(displayName)}</strong><small>${type} · ${uuid}</small></div></div><div class="account-item-meta"><span class="account-state">${item.active ? "Active" : "Available"}</span>${shellButton("Manage", "open-accounts", "quiet")}</div></div>`;
    }).join("") : `<div class="empty-note account-empty"><div><h2>No accounts connected</h2><p class="subtle">Microsoft OAuth and offline profiles remain managed by the native secure account manager.</p>${shellButton("Open account manager", "open-accounts", "primary")}</div></div>`;

    workspace.innerHTML = `
      <section class="page"><div class="page-heading"><div><p class="eyebrow">Identity</p><h1>Accounts</h1><p class="lead">Your real account list, profile UUID and skin face stay synchronized with the native account manager.</p></div>${shellButton("Open account manager", "open-accounts", "primary")}</div>
      <div class="account-hero"><div class="account-summary"><div class="big-avatar">${hasAccount ? (accounts.find((item) => item.active)?.face ? `<img class="account-face" src="${escapeHtml(accounts.find((item) => item.active).face)}" alt="Active account face">` : escapeHtml((account || "C").slice(0, 1).toUpperCase())) : "C"}</div><div><strong>${hasAccount ? escapeHtml(account) : "No active account"}</strong><p class="subtle">${hasAccount ? "Active Minecraft profile · face loaded from UUID" : "Connect an account to launch licensed instances"}</p></div></div><span class="subtle">Auth stays native</span></div>
      <div class="account-list">${accountCards}</div></section>`;
  };

  const renderSettings = () => {
    const themeCards = Object.entries(weatherThemes).map(([key, theme]) => `<button class="weather-theme-card ${readWeatherTheme() === key ? "is-selected" : ""}" data-weather-theme="${key}"><span class="theme-swatch theme-${key}"><span class="theme-swatch-icon"></span></span><span class="theme-card-copy"><strong>${theme.label}</strong><small>${theme.description}</small></span><span class="theme-check" aria-hidden="true"></span></button>`).join("");
    const snowVariant = readSnowVariant();
    workspace.innerHTML = `
      <section class="page"><div class="page-heading"><div><p class="eyebrow">Preferences</p><h1>Settings</h1><p class="lead">Shape Cloudy around your workflow while retaining every existing preference and custom theme.</p></div>${shellButton("Open full settings", "open-settings", "primary")}</div>
      <div class="settings-grid"><nav class="settings-nav"><button class="is-active" type="button">General</button><button type="button">Appearance</button><button type="button">Minecraft</button><button type="button">Java</button><button type="button">Services</button><button type="button">Proxy</button></nav><div class="settings-panel"><div class="setting-line"><div><strong>Cloudy workspace</strong><small>Keep pages together in one connected frame</small></div><button class="toggle is-on" aria-label="Cloudy workspace enabled"></button></div><div class="setting-line"><div><strong>Native custom themes</strong><small>Existing ThemeManager and user theme files remain respected</small></div><button class="toggle is-on" aria-label="Custom themes enabled"></button></div><div class="theme-section"><div class="theme-section-heading"><div><strong>Weather atmosphere</strong><small>Cloudy is the default. Choose a scene; motion respects reduced-motion settings.</small></div><span class="theme-current">${weatherThemes[readWeatherTheme()].label}</span></div><div class="weather-theme-grid">${themeCards}</div><div class="snow-variant-row"><span>Snow tone</span><button class="snow-variant-button ${snowVariant === "light" ? "is-selected" : ""}" data-snow-variant="light">Light</button><button class="snow-variant-button ${snowVariant === "dark" ? "is-selected" : ""}" data-snow-variant="dark">Dark</button></div></div><div class="native-handoff"><span>Advanced and provider-specific settings open in the full native page.</span>${shellButton("Open settings", "open-settings", "quiet")}</div></div></div></section>`;
  };

  const renderSkins = ({ hasAccount }) => {
    const action = hasAccount ? "open-skins" : "open-accounts";
    const actionLabel = hasAccount ? "Open Skin Studio" : "Connect Microsoft account";
    const subtitle = hasAccount ? "Preview, import and manage skins with the existing 3D model, cape and account workflow." : "Connect a Microsoft account first to use the secure skin and cape workflow.";
    workspace.innerHTML = `
      <section class="page"><div class="page-heading"><div><p class="eyebrow">Appearance</p><h1>Skin Studio</h1><p class="lead">${subtitle}</p></div>${shellButton(actionLabel, action, "primary")}</div>
      <div class="skin-grid"><div class="skin-preview"><div class="skin-cloud" aria-hidden="true"></div></div><div class="feature-panel"><p class="eyebrow">Cloud-ready appearance</p><h2>A calmer way to manage your look.</h2><p class="lead">Import a file, fetch by nickname or use a direct URL. The native preview remains responsible for model and cape rendering.</p><div class="native-handoff"><span>${hasAccount ? "Open full preview and import controls." : "Licensed skin controls stay behind Microsoft OAuth."}</span>${shellButton(actionLabel, action, "primary")}</div></div></div></section>`;
  };

  function render() {
    const currentState = state();
    applyPalette(currentState.palette);
    if (route === "new") renderNew(currentState);
    else if (route === "mods") renderMods(currentState);
    else if (route === "files") renderFiles(currentState);
    else if (route === "accounts") renderAccounts(currentState);
    else if (route === "settings") renderSettings();
    else if (route === "skins") renderSkins(currentState);
    else renderHome(currentState);

    const activeAccount = currentState.accounts.find((item) => item.active) || currentState.accounts[0];
    const running = currentState.instances.filter((item) => item.running).length;
    topbarRunningText.textContent = running ? `${running} ${running === 1 ? "instance is" : "instances are"} currently running` : "No instances are currently running";
    profileName.textContent = currentState.hasAccount ? currentState.account : "No account";
    profileAvatarLetter.textContent = (currentState.account || "C").slice(0, 1).toUpperCase();
    if (activeAccount?.face) {
      profileAvatarImage.src = activeAccount.face;
      profileAvatarImage.alt = `${activeAccount.name || currentState.account} face`;
      profileAvatarImage.hidden = false;
      profileAvatarLetter.hidden = true;
    } else {
      profileAvatarImage.removeAttribute("src");
      profileAvatarImage.hidden = true;
      profileAvatarLetter.hidden = false;
    }
  }

  document.addEventListener("click", (event) => {
    const routeButton = event.target.closest("[data-route]");
    if (routeButton) {
      setRoute(routeButton.dataset.route);
      return;
    }

    const themeButton = event.target.closest("[data-weather-theme]");
    if (themeButton) {
      chooseWeatherTheme(themeButton.dataset.weatherTheme);
      event.stopPropagation();
      return;
    }

    const snowButton = event.target.closest("[data-snow-variant]");
    if (snowButton) {
      try { localStorage.setItem("cloudy-snow-variant", snowButton.dataset.snowVariant); } catch (_) {}
      document.documentElement.dataset.snowVariant = snowButton.dataset.snowVariant;
      render();
      event.stopPropagation();
      return;
    }

    const actionButton = event.target.closest("[data-action]");
    if (actionButton) {
      const action = actionButton.dataset.action;
      const card = actionButton.closest("[data-instance-id]");
      const id = card?.dataset.instanceId || state().instances[0]?.id || "";
      if (action === "home") setRoute("home");
      if (action === "new") setRoute("new");
      if (action === "open-new") { setRoute("new"); callNative("openNewInstance"); }
      if (action === "accounts") setRoute("accounts");
      if (action === "open-accounts") { setRoute("accounts"); callNative("openAccounts"); }
      if (action === "settings") setRoute("settings");
      if (action === "open-settings") { setRoute("settings"); callNative("openSettings"); }
      if (action === "skins") setRoute("skins");
      if (action === "open-skins") { setRoute("skins"); callNative("openSkinStudio"); }
      if (action === "mods") setRoute("mods");
      if (action === "open-mods") { setRoute("mods"); callNative("openMods", id); }
      if (action === "files") setRoute("files");
      if (action === "open-files") { setRoute("files"); callNative("openFiles", id); }
      if (action === "window-minimize") { callNative("minimizeWindow"); }
      if (action === "window-toggle-maximize") { callNative("toggleMaximizeWindow"); }
      if (action === "window-close") { callNative("closeWindow"); }
      if (action === "launch") { callNative("launchInstance", id); }
      if (action === "refresh") { callNative("refreshInstances"); }
      event.stopPropagation();
      return;
    }

    const card = event.target.closest("[data-instance-id]");
    if (card) {
      callNative("selectInstance", card.dataset.instanceId);
      setRoute("home");
    }
  });

  const topbar = document.querySelector(".topbar");
  topbar?.addEventListener("pointerdown", (event) => {
    if (event.button === 0 && !event.target.closest("button")) callNative("beginWindowDrag");
  });

  document.addEventListener("input", (event) => {
    if (event.target.id !== "instance-search") return;
    const query = event.target.value.trim().toLowerCase();
    document.querySelectorAll(".instance-card").forEach((card) => {
      card.hidden = query && !card.textContent.toLowerCase().includes(query);
    });
  });

  navButtons.forEach((button) => button.addEventListener("keydown", (event) => {
    if (event.key === "Enter" || event.key === " ") button.click();
  }));

  applyWeatherTheme();

  const weatherInteraction = document.createElement("div");
  weatherInteraction.className = "weather-interaction";
  weatherInteraction.setAttribute("aria-hidden", "true");
  document.body.append(weatherInteraction);
  let lastSnowBrush = 0;
  document.addEventListener("pointermove", (event) => {
    if (document.documentElement.dataset.weatherTheme !== "snow" || event.clientY < 48 || event.timeStamp - lastSnowBrush < 90) return;
    lastSnowBrush = event.timeStamp;
    const brush = document.createElement("i");
    brush.className = "snow-brush";
    brush.style.left = `${event.clientX - 7}px`;
    brush.style.top = `${event.clientY - 7}px`;
    weatherInteraction.append(brush);
    window.setTimeout(() => brush.remove(), 720);
    while (weatherInteraction.childElementCount > 18) weatherInteraction.firstElementChild.remove();
  });

  if (window.qt?.webChannelTransport && window.QWebChannel) {
    new QWebChannel(qt.webChannelTransport, (channel) => {
      bridge = channel.objects.cloudy;
      if (bridge?.stateChanged) bridge.stateChanged.connect(render);
      render();
    });
  } else {
    render();
  }
})();
