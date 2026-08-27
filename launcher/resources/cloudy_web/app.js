(() => {
  "use strict";

  const workspace = document.getElementById("workspace");
  const crumb = document.getElementById("crumb");
  const profileName = document.getElementById("profile-name");
  const profileSubtitle = document.getElementById("profile-subtitle");
  const profileAvatar = document.getElementById("profile-avatar");
  const profileAvatarLetter = document.getElementById("profile-avatar-letter");
  const profileAvatarImage = document.getElementById("profile-avatar-image");
  const topbarRunningText = document.getElementById("topbar-running-text");
  const topbarRunningCard = document.getElementById("topbar-running-card");
  const navButtons = [...document.querySelectorAll(".rail-button[data-route]")];
  let bridge = null;
  let route = "home";
  let selectedInstanceOverride = "";
  let onboardingStep = 1;
  let onboardingAuth = "";
  let onboardingNickname = "";

  const onboardingStorageKey = "cloudy-onboarding-step";
  const readOnboardingStep = () => {
    try { return Math.min(5, Math.max(1, Number(localStorage.getItem(onboardingStorageKey) || 1))); } catch (_) { return 1; }
  };
  const saveOnboardingStep = (step) => {
    onboardingStep = Math.min(5, Math.max(1, step));
    try { localStorage.setItem(onboardingStorageKey, String(onboardingStep)); } catch (_) {}
  };
  const uiLanguage = () => {
    try {
      const localLanguage = localStorage.getItem("cloudy-language");
      if (localLanguage === "ru" || localLanguage === "en") return localLanguage;
    } catch (_) {}
    const nativeLanguage = bridge?.language;
    return nativeLanguage?.toLowerCase().startsWith("ru") ? "ru" : "en";
  };
  const copy = (en, ru) => uiLanguage() === "ru" ? ru : en;
  const formatBytes = (value) => {
    const bytes = Number(value || 0);
    if (!bytes) return copy("Not measured yet", "Пока не измерено");
    const units = ["B", "KB", "MB", "GB", "TB"];
    const index = Math.min(units.length - 1, Math.floor(Math.log(bytes) / Math.log(1024)));
    return `${(bytes / Math.pow(1024, index)).toFixed(index ? 1 : 0)} ${units[index]}`;
  };
  onboardingStep = readOnboardingStep();

  const weatherThemes = {
    cloudy: { label: "Cloudy", labelRu: "Облачная", description: "Soft clouds, sun breaks, leafy trees and distant mountains", descriptionRu: "Облака, солнечные просветы, деревья и горы" },
    rain: { label: "Rain", labelRu: "Дождь", description: "Fine rain, pale clouds and a few quiet puddles", descriptionRu: "Мелкий дождь, серые облака и небольшие лужи" },
    storm: { label: "Storm", labelRu: "Гроза", description: "Heavy rain, dark clouds, fog, wind and large puddles", descriptionRu: "Сильный дождь, тучи, туман, ветер и большие лужи" },
    sunny: { label: "Sunny", labelRu: "Солнечная", description: "A bright summer sky with warm light and white clouds", descriptionRu: "Светлое летнее небо, солнце и белые облака" },
    night: { label: "Night", labelRu: "Ночная", description: "Stars, constellations and a slow midnight sky", descriptionRu: "Звёзды, созвездия и спокойное ночное небо" },
    snow: { label: "Snow", labelRu: "Снежная", description: "White fog, falling snow, bare trees and snowy mountains", descriptionRu: "Белый туман, снег, голые деревья и снежные горы" },
  };

  const readWeatherTheme = () => {
    try {
      const saved = localStorage.getItem("cloudy-weather-theme");
      if (weatherThemes[saved]) return saved;
    } catch (_) {}
    const nativeTheme = bridge?.weatherTheme;
    return weatherThemes[nativeTheme] ? nativeTheme : "cloudy";
  };

  const readSnowVariant = () => {
    try {
      const saved = localStorage.getItem("cloudy-snow-variant");
      if (saved === "dark" || saved === "light") return saved;
    } catch (_) {}
    return bridge?.snowVariant === "dark" ? "dark" : "light";
  };

  const applyWeatherTheme = (theme = readWeatherTheme()) => {
    const safeTheme = weatherThemes[theme] ? theme : "cloudy";
    document.documentElement.dataset.weatherTheme = safeTheme;
    document.documentElement.dataset.snowVariant = readSnowVariant();
    return safeTheme;
  };

  let uiAudioContext = null;
  const playStyleTone = (theme) => {
    if (bridge?.soundsEnabled === false) return;
    try {
      uiAudioContext ||= new (window.AudioContext || window.webkitAudioContext)();
      const frequencies = { cloudy: 392, rain: 330, storm: 196, sunny: 523, night: 262, snow: 440 };
      const oscillator = uiAudioContext.createOscillator();
      const gain = uiAudioContext.createGain();
      oscillator.type = theme === "storm" ? "triangle" : "sine";
      oscillator.frequency.value = frequencies[theme] || frequencies.cloudy;
      gain.gain.setValueAtTime(0.0001, uiAudioContext.currentTime);
      gain.gain.exponentialRampToValueAtTime(0.035, uiAudioContext.currentTime + 0.025);
      gain.gain.exponentialRampToValueAtTime(0.0001, uiAudioContext.currentTime + 0.19);
      oscillator.connect(gain).connect(uiAudioContext.destination);
      oscillator.start();
      oscillator.stop(uiAudioContext.currentTime + 0.21);
    } catch (_) {}
  };

  const chooseWeatherTheme = (theme) => {
    const safeTheme = applyWeatherTheme(theme);
    try { localStorage.setItem("cloudy-weather-theme", safeTheme); } catch (_) {}
    callNative("setWeatherTheme", safeTheme);
    playStyleTone(safeTheme);
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
      firstRun: Boolean(bridge?.firstRun),
      palette: bridge?.paletteData || {},
      storage: bridge?.storageData || {},
      language: uiLanguage(),
      globalMaxMemory: Number(bridge?.globalMaxMemory || 4096),
      selectedInstanceId: selectedInstanceOverride || bridge?.selectedInstanceId || "",
      soundsEnabled: bridge?.soundsEnabled !== false,
      weatherTheme: readWeatherTheme(),
      snowVariant: readSnowVariant(),
    };
  };

  const applyPalette = (palette) => {
    const root = document.documentElement;
    const snowLight = root.dataset.weatherTheme === "snow" && root.dataset.snowVariant === "light";
    if (snowLight) {
      root.style.removeProperty("--text");
      root.style.removeProperty("--white");
      return;
    }
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
    const labels = { home: ["Home", "Главная"], new: ["New instance", "Новая сборка"], accounts: ["Accounts", "Аккаунты"], settings: ["Settings", "Настройки"], skins: ["Skin Studio", "Скины"], mods: ["Mods", "Моды"], files: ["Files", "Файлы"] };
    crumb.textContent = copy(...(labels[route] || labels.home));
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

  const renderOnboarding = (currentState) => {
    const step = onboardingStep;
    const stepLabels = uiLanguage() === "ru" ? ["Язык", "Стиль", "Java", "Диск", "Аккаунт"] : ["Language", "Style", "Java", "Storage", "Account"];
    const selectedTheme = currentState.weatherTheme || "cloudy";
    const styleCards = Object.entries(weatherThemes).map(([key, theme]) => `<button class="onboarding-style-card ${selectedTheme === key ? "is-selected" : ""}" data-onboard-action="style" data-value="${key}"><span class="theme-swatch theme-${key}"><span class="theme-swatch-icon"></span></span><span><strong>${uiLanguage() === "ru" ? ({cloudy:"Облачная",rain:"Дождь",storm:"Гроза",sunny:"Солнечная",night:"Ночная",snow:"Снежная"}[key]) : theme.label}</strong><small>${uiLanguage() === "ru" ? ({cloudy:"Облака, солнце и горы",rain:"Мелкий дождь и лужи",storm:"Тучи, туман и сильный дождь",sunny:"Светлое небо и солнце",night:"Звёзды и созвездия",snow:"Снег и белый туман"}[key]) : theme.description}</small></span><i class="selection-dot" aria-hidden="true"></i></button>`).join("");
    const storage = currentState.storage || {};
    const language = currentState.language === "ru" ? "ru" : "en";
    const ru = language === "ru";
    let content = "";
    if (step === 1) {
      content = `<div class="onboarding-step-copy"><p class="eyebrow">01 / ${ru ? "Начало" : "Start"}</p><h2>${ru ? "Добро пожаловать в Cloudy Launcher" : "Welcome to Cloudy Launcher"}</h2><p class="lead">${ru ? "Выберите язык интерфейса. Его выбор сохраняется в настройках лаунчера." : "Choose the interface language. Your choice is saved in the launcher settings."}</p><div class="onboarding-choice-grid"><button class="onboarding-choice ${language === "ru" ? "is-selected" : ""}" data-onboard-action="language" data-value="ru"><strong>Русский</strong><small>Русский интерфейс</small></button><button class="onboarding-choice ${language === "en" ? "is-selected" : ""}" data-onboard-action="language" data-value="en"><strong>English</strong><small>English interface</small></button></div></div>`;
    } else if (step === 2) {
      content = `<div class="onboarding-step-copy"><p class="eyebrow">02 / ${ru ? "Внешний вид" : "Appearance"}</p><h2>${ru ? "Выберите стиль Cloudy" : "Choose a Cloudy style"}</h2><p class="lead">${ru ? "Это атмосферный слой Cloudy. Native custom themes и системная тема сохраняются отдельно." : "This is Cloudy’s atmospheric layer. Native custom themes and the system theme remain separate."}</p><div class="onboarding-style-grid">${styleCards}</div></div>`;
    } else if (step === 3) {
      const memory = Math.max(1024, Math.min(16384, currentState.globalMaxMemory || 4096));
      content = `<div class="onboarding-step-copy"><p class="eyebrow">03 / ${ru ? "Память Java" : "Java memory"}</p><h2>${ru ? "Сколько памяти дать Minecraft?" : "How much memory should Minecraft use?"}</h2><p class="lead">${ru ? "Значение записывается в глобальные MinMemAlloc/MaxMemAlloc и используется при запуске сборок." : "This writes the global MinMemAlloc/MaxMemAlloc values used by launched instances."}</p><div class="memory-value"><strong>${memory} MB</strong><span>${ru ? "По умолчанию 4096 MB" : "4096 MB by default"}</span></div><input class="memory-slider" id="onboard-memory" type="range" min="1024" max="16384" step="512" value="${memory}" aria-label="Java memory in megabytes"><div class="range-labels"><span>1024 MB</span><span>16384 MB</span></div><div class="callout">${ru ? "Это глобальная настройка. Отдельная сборка может иметь собственное override-значение в настройках экземпляра." : "This is the global setting. An individual instance may still have its own override in instance settings."}</div></div>`;
    } else if (step === 4) {
      content = `<div class="onboarding-step-copy"><p class="eyebrow">04 / ${ru ? "Установка" : "Storage"}</p><h2>${ru ? "Проверьте место на диске" : "Review disk usage"}</h2><p class="lead">${ru ? "Cloudy показывает реальный размер доступного приложения и данных. Никакие пути не передаются в web layer." : "Cloudy reports measured application and data sizes. File-system paths never enter the web layer."}</p><div class="storage-grid"><div class="storage-card"><span class="storage-icon storage-app-icon"></span><strong>${formatBytes(storage.applicationBytes)}</strong><small>${ru ? "Приложение Cloudy" : "Cloudy application"}</small></div><div class="storage-card"><span class="storage-icon storage-data-icon"></span><strong>${formatBytes(storage.dataBytes)}</strong><small>${ru ? "Данные, сборки и кэш" : "Data, instances and cache"}</small></div><div class="storage-card"><span class="storage-icon storage-disk-icon"></span><strong>${formatBytes(storage.diskAvailableBytes)}</strong><small>${ru ? "Свободно на диске" : "Available on disk"}</small></div></div><div class="callout">${ru ? "Иконка Cloudy и размер установочного набора находятся в установщике. Здесь показывается размер текущего native application root и data root." : "The Cloudy icon and setup package size belong to the installer. This page measures the current native application root and data root."}</div></div>`;
    } else {
      const accountReady = currentState.hasAccount;
      content = `<div class="onboarding-step-copy"><p class="eyebrow">05 / ${ru ? "Авторизация" : "Account"}</p><h2>${ru ? "Выберите способ входа" : "Choose how to continue"}</h2><p class="lead">${ru ? "Microsoft сохраняет лицензионный вход через настоящий OAuth/device-code flow. Никнейм создаёт локальный offline-профиль без лицензии." : "Microsoft uses the real OAuth/device-code flow. A nickname creates a local offline profile without a license."}</p><div class="auth-choice-grid"><button class="auth-choice auth-choice-microsoft ${onboardingAuth === "microsoft" || accountReady ? "is-selected" : ""}" data-onboard-action="microsoft"><span class="auth-icon auth-microsoft"><i></i><i></i><i></i><i></i></span><span><strong>${ru ? "Microsoft аккаунт" : "Microsoft account"}</strong><small>${accountReady ? (ru ? "Аккаунт подключён" : "Account connected") : (ru ? "OAuth и лицензия" : "OAuth and license")}</small></span></button><button class="auth-choice auth-choice-nickname ${onboardingAuth === "nickname" ? "is-selected" : ""}" data-onboard-action="nickname"><span class="auth-icon auth-shirt"></span><span><strong>${ru ? "По никнейму" : "Nickname"}</strong><small>${ru ? "Только локальная игра" : "Local/offline play"}</small></span></button></div>${onboardingAuth === "nickname" ? `<div class="nickname-form"><label for="onboard-nickname">${ru ? "Никнейм (3–16 символов)" : "Nickname (3–16 characters)"}</label><div class="inline-form"><input class="form-input" id="onboard-nickname" maxlength="16" pattern="[A-Za-z0-9_]{3,16}" value="${escapeHtml(onboardingNickname)}" placeholder="CloudyPlayer"><button class="action-button primary" data-onboard-action="create-nickname">${ru ? "Создать профиль" : "Create profile"}</button></div><small>${ru ? "Локальный профиль сохраняется в native accounts.json и доступен после перезапуска." : "The local profile is saved by the native account manager and remains after restart."}</small></div>` : ""}<div class="callout">${accountReady ? (ru ? "Профиль готов. Лицо скина будет взято native account manager из UUID, если профиль предоставляет его." : "Your profile is ready. The native account manager supplies the UUID skin face when available.") : (ru ? "Выберите Microsoft или никнейм. Можно продолжить позже, не подменяя реальные учётные данные." : "Choose Microsoft or nickname. You can continue later without fabricating credentials.")}</div></div>`;
    }
    const stepper = stepLabels.map((label, index) => `<span class="onboarding-step ${index + 1 === step ? "is-current" : index + 1 < step ? "is-done" : ""}"><b>${String(index + 1).padStart(2, "0")}</b><small>${label}</small></span>`).join("");
    workspace.innerHTML = `<section class="page onboarding-page"><div class="onboarding-shell"><div class="onboarding-header"><div><p class="eyebrow">Cloudy setup</p><h1>${ru ? "Настройте рабочее пространство" : "Set up your workspace"}</h1><p class="lead">${ru ? "Пять коротких шагов. Настройки записываются native лаунчером и не исчезают после выхода." : "Five short steps. Choices are written by the native launcher and survive restart."}</p></div><span class="onboarding-badge">${String(step).padStart(2, "0")} / 05</span></div><div class="onboarding-stepper">${stepper}</div><div class="onboarding-content">${content}</div><div class="onboarding-footer"><button class="action-button quiet" data-onboard-action="back" ${step === 1 ? "disabled" : ""}>${ru ? "Назад" : "Back"}</button><span class="onboarding-footer-note">${ru ? "Можно изменить позже в настройках" : "You can change these later in Settings"}</span><button class="action-button primary" data-onboard-action="next">${step === 5 ? (currentState.hasAccount ? (ru ? "Открыть Cloudy" : "Open Cloudy") : (ru ? "Продолжить позже" : "Continue later")) : (ru ? "Далее" : "Continue")}</button></div></div></section>`;
  };

  const renderHome = ({ instances, account, hasAccount, firstRun, accounts }) => {
    const running = instances.filter((item) => item.running).length;
    const runningText = running ? `${running} ${running === 1 ? "instance is" : "instances are"} currently running` : "No instances are currently running";
    const cards = instances.map((item) => `
      <article class="instance-card" data-instance-id="${escapeHtml(item.id)}" tabindex="0" aria-label="Open ${escapeHtml(item.name)}">
        <div class="instance-art">
          ${item.iconData ? `<img src="${escapeHtml(item.iconData)}" alt="${escapeHtml(item.name)} icon">` : `<div class="instance-placeholder">${escapeHtml((item.name || "C").slice(0, 1).toUpperCase())}</div>`}
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

    if (firstRun) {
      renderOnboarding({ instances, account, hasAccount, firstRun, accounts, storage: state().storage, language: state().language, globalMaxMemory: state().globalMaxMemory, soundsEnabled: state().soundsEnabled, weatherTheme: state().weatherTheme, snowVariant: state().snowVariant });
      return;
    }

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

  const renderMods = ({ instances, selectedInstanceId }) => {
    const selected = instances.find((item) => item.id === selectedInstanceId) || instances[0];
    workspace.innerHTML = `
      <section class="page"><div class="page-heading"><div><p class="eyebrow">Resource workspace</p><h1>Mods & resources</h1><p class="lead">Search and install mods through the existing Modrinth, CurseForge and local resource workflows.</p></div>${shellButton("Back to library", "home", "quiet")}</div>
      <div class="hero-strip"><div class="hero-panel"><p class="eyebrow">Selected instance</p><h2>${selected ? escapeHtml(selected.name) : "No instance selected"}</h2><p class="lead">Select an instance from Library, then open its real mod page with filters, versions and install tasks.</p>${selected ? shellButton("Open native mod workspace", "open-mods", "primary") : shellButton("Choose an instance", "home", "quiet")}</div><div class="side-panel"><span>Sources</span><strong>3+</strong><span>Modrinth · CurseForge · local files</span></div></div>
      <div class="flow-layout"><div class="feature-panel"><div class="tools-row"><h2>Install source</h2><span class="subtle">Task-backed</span></div><div class="provider-list"><button class="provider-card" data-action="open-mods"><span class="provider-logo provider-modrinth"><img src="data:image/png;base64,CLOUDY_PROVIDER_MODRINTH" alt=""></span><span><strong>Modrinth</strong><small>Search projects and versions</small></span></button><button class="provider-card" data-action="open-mods"><span class="provider-logo provider-curseforge"><img src="data:image/png;base64,CLOUDY_PROVIDER_CURSEFORGE" alt=""></span><span><strong>CurseForge</strong><small>Use existing package metadata</small></span></button></div></div><aside class="side-panel"><p class="eyebrow">Provider details</p><h2>Authors stay linked</h2><p class="subtle">Open the native resource workspace for real project metadata, clickable Modrinth and CurseForge author links, versions and install tasks.</p><div class="callout">Author pages, project links and provider extras are loaded from the existing native models. Cloudy does not invent author profiles or expose provider credentials.</div></aside></div></section>`;
  };

  const renderFiles = ({ instances, selectedInstanceId }) => {
    const selected = instances.find((item) => item.id === selectedInstanceId) || instances[0];
    workspace.innerHTML = `
      <section class="page"><div class="page-heading"><div><p class="eyebrow">Workspace files</p><h1>Files</h1><p class="lead">Open the selected instance folder with the native file model. No path is exposed to the web layer.</p></div>${shellButton("Back to library", "home", "quiet")}</div>
      <div class="empty-note"><div><h2>${selected ? `Open files for ${escapeHtml(selected.name)}` : "Select an instance first"}</h2><p class="subtle">Drag & drop and editing continue in the native, permission-aware file workflow.</p><div style="margin-top:16px">${selected ? shellButton("Open instance folder", "open-files", "primary") : shellButton("Go to library", "home", "quiet")}</div></div></div></section>`;
  };

  const renderInstance = ({ instances, selectedInstanceId }) => {
    const selected = instances.find((item) => item.id === selectedInstanceId) || instances[0];
    if (!selected) {
      workspace.innerHTML = `<section class="page"><div class="page-heading"><div><p class="eyebrow">Instance workspace</p><h1>No instance selected</h1><p class="lead">Choose a build from Library to open its overview.</p></div>${shellButton("Back to library", "home", "primary")}</div><div class="empty-note"><h2>Your library is empty</h2><p class="subtle">Create or import a Minecraft instance first.</p></div></section>`;
      return;
    }
    const status = selected.running ? "Playing" : "Ready to play";
    const icon = selected.iconData ? `<img class="instance-detail-icon" src="${escapeHtml(selected.iconData)}" alt="${escapeHtml(selected.name)} icon">` : `<span class="instance-detail-placeholder">${escapeHtml((selected.name || "C").slice(0, 1).toUpperCase())}</span>`;
    workspace.innerHTML = `<section class="page instance-detail-page"><div class="page-heading"><div><p class="eyebrow">Instance workspace</p><h1>${escapeHtml(selected.name)}</h1><p class="lead">A connected overview for this Minecraft build. Launching and resource changes remain handled by the native task pipeline.</p></div>${shellButton("Back to library", "home", "quiet")}</div><div class="instance-detail-hero"><div class="instance-detail-identity"><div class="instance-detail-art">${icon}</div><div><span class="status-chip"><span class="status-dot"></span>${status}</span><h2>${escapeHtml(selected.name)}</h2><p class="subtle">${escapeHtml(selected.managed ? `${selected.managedType || "Managed"} · ${selected.managedVersion || "version"}` : "Local Minecraft instance")}</p></div></div><div class="instance-detail-actions">${shellButton(selected.running ? "Open" : "Launch", "launch", "primary", selected.id)}${shellButton("Mods", "open-mods", "quiet", selected.id)}${shellButton("Files", "open-files", "quiet", selected.id)}</div></div><div class="detail-grid"><div class="feature-panel"><p class="eyebrow">Overview</p><h2>Build, inspect and play</h2><p class="subtle">Use the native instance page for version details, logs, screenshots, worlds and task progress.</p>${shellButton("Open native instance page", "open-native-instance", "quiet", selected.id)}</div><aside class="side-panel"><p class="eyebrow">Safe handoff</p><strong>Native workflow connected</strong><p class="subtle">Cloudy keeps account, file and installer permissions in the native layer.</p></aside></div></section>`;
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

  const renderSettings = (currentState = state()) => {
    const themeCards = Object.entries(weatherThemes).map(([key, theme]) => `<button class="weather-theme-card ${readWeatherTheme() === key ? "is-selected" : ""}" data-weather-theme="${key}"><span class="theme-swatch theme-${key}"><span class="theme-swatch-icon"></span></span><span class="theme-card-copy"><strong>${copy(theme.label, theme.labelRu)}</strong><small>${copy(theme.description, theme.descriptionRu)}</small></span><span class="theme-check" aria-hidden="true"></span></button>`).join("");
    const snowVariant = readSnowVariant();
    workspace.innerHTML = `
      <section class="page"><div class="page-heading"><div><p class="eyebrow">Preferences</p><h1>Settings</h1><p class="lead">Shape Cloudy around your workflow while retaining every existing preference and custom theme.</p></div>${shellButton("Open full settings", "open-settings", "primary")}</div>
      <div class="settings-grid"><nav class="settings-nav"><button class="is-active" type="button" data-action="open-settings"><span class="nav-mini-icon icon-home"></span>General</button><button type="button" data-action="open-settings"><span class="nav-mini-icon icon-sun"></span>Appearance</button><button type="button" data-action="open-settings"><span class="nav-mini-icon icon-cube"></span>Minecraft</button><button type="button" data-action="open-settings"><span class="nav-mini-icon icon-memory"></span>Java</button><button type="button" data-action="open-settings"><span class="nav-mini-icon icon-cloud"></span>Services</button><button type="button" data-action="open-settings"><span class="nav-mini-icon icon-shield"></span>Proxy</button></nav><div class="settings-panel"><div class="setting-line"><div class="setting-title"><span class="setting-icon icon-grid"></span><span><strong>Cloudy workspace</strong><small>Keep pages together in one connected frame</small></span></div><button class="toggle is-on" aria-label="Cloudy workspace enabled"></button></div><div class="setting-line"><div class="setting-title"><span class="setting-icon icon-palette"></span><span><strong>Native custom themes</strong><small>Existing ThemeManager and user theme files remain respected</small></span></div><button class="toggle is-on" aria-label="Custom themes enabled"></button></div><div class="setting-line"><div class="setting-title"><span class="setting-icon icon-person"></span><span><strong>Account profile</strong><small>${currentState.hasAccount ? escapeHtml(currentState.account) : "No active account"}</small></span></div>${shellButton("Manage", "open-accounts", "quiet")}</div><div class="setting-line"><div class="setting-title"><span class="setting-icon icon-memory"></span><span><strong>Global Java memory</strong><small>${currentState.globalMaxMemory} MB max heap used by new launches</small></span></div>${shellButton("Open Java settings", "open-settings", "quiet")}</div><div class="setting-line"><div class="setting-title"><span class="setting-icon icon-sound"></span><span><strong>Atmosphere sounds</strong><small>Subtle Cloudy feedback sounds; never required for gameplay</small></span></div><button class="toggle ${currentState.soundsEnabled ? "is-on" : ""}" data-action="toggle-sounds" aria-label="Toggle atmosphere sounds"></button></div><div class="theme-section"><div class="theme-section-heading"><div><strong>Weather atmosphere</strong><small>Cloudy is the default. Choose a scene; motion respects reduced-motion settings.</small></div><span class="theme-current">${copy(weatherThemes[readWeatherTheme()].label, weatherThemes[readWeatherTheme()].labelRu)}</span></div><div class="weather-theme-grid">${themeCards}</div><div class="snow-variant-row"><span>Snow tone</span><button class="snow-variant-button ${snowVariant === "light" ? "is-selected" : ""}" data-snow-variant="light">Light</button><button class="snow-variant-button ${snowVariant === "dark" ? "is-selected" : ""}" data-snow-variant="dark">Dark</button></div></div><div class="native-handoff"><span>Advanced provider, account and appearance options remain in the full native pages.</span>${shellButton("Open full settings", "open-settings", "quiet")}</div></div></div></section>`;
  };

  const renderSkins = ({ hasAccount }) => {
    const action = hasAccount ? "open-skins" : "open-accounts";
    const actionLabel = hasAccount ? "Open Skin Studio" : "Connect Microsoft account";
    const subtitle = hasAccount ? "Preview, import and manage skins with the existing 3D model, cape and account workflow." : "Connect a Microsoft account first to use the secure skin and cape workflow.";
    workspace.innerHTML = `
      <section class="page"><div class="page-heading"><div><p class="eyebrow">Appearance</p><h1>Skin Studio</h1><p class="lead">${subtitle}</p></div>${shellButton(actionLabel, action, "primary")}</div>
      <div class="skin-grid"><div class="skin-preview"><div class="skin-cloud" aria-hidden="true"></div></div><div class="feature-panel"><p class="eyebrow">Cloud-ready appearance</p><h2>A calmer way to manage your look.</h2><p class="lead">Import a file, fetch by nickname or use a direct URL. The native preview remains responsible for model and cape rendering.</p><div class="native-handoff"><span>${hasAccount ? "Open full preview and import controls." : "Licensed skin controls stay behind Microsoft OAuth."}</span>${shellButton(actionLabel, action, "primary")}</div></div></div></section>`;
  };

  const translateVisibleUi = () => {
    if (uiLanguage() !== "ru") return;
    const translations = {
      "Cloud workspace": "Облачное рабочее пространство", "Greetings!": "Добро пожаловать!", "Here are your worlds and servers. Everything stays in one calm workspace.": "Здесь находятся ваши миры и серверы. Всё остаётся в одном рабочем пространстве.", "Search instances": "Поиск сборок", "Create instance": "Создать сборку", "Build, browse and play without leaving the flow.": "Создавайте, просматривайте и запускайте, не покидая рабочий процесс.", "Active profile": "Активный профиль", "Library": "Библиотека", "Refresh": "Обновить", "Grid": "Сетка", "Your library is clear. Add a Minecraft instance to start shaping your cloud.": "Библиотека пуста. Добавьте сборку Minecraft, чтобы начать работу.", "Instance builder": "Создание сборки", "Create an instance": "Создать сборку", "Choose a source": "Выберите источник", "All providers": "Все провайдеры", "Starting point": "Начальная точка", "Pick where the pack comes from": "Выберите источник сборки", "Open builder": "Открыть мастер", "Build preview": "Предпросмотр сборки", "Ready for the native flow": "Готово к native-процессу", "Resource workspace": "Рабочее пространство модов", "Mods & resources": "Моды и ресурсы", "Selected instance": "Выбранная сборка", "No instance selected": "Сборка не выбрана", "Authors stay linked": "Авторы остаются доступны", "Workspace files": "Файлы рабочей области", "Select an instance first": "Сначала выберите сборку", "Identity": "Профиль", "Accounts": "Аккаунты", "No accounts connected": "Аккаунты не подключены", "Open account manager": "Открыть менеджер аккаунтов", "Preferences": "Настройки", "Settings": "Настройки", "Appearance": "Внешний вид", "Minecraft": "Minecraft", "Java": "Java", "Services": "Сервисы", "Proxy": "Прокси", "General": "Общие", "Skin Studio": "Студия скинов", "Open Skin Studio": "Открыть студию скинов", "No active account": "Активный аккаунт не выбран", "Manage": "Управление", "Open full settings": "Открыть полные настройки", "Light": "Светлая", "Dark": "Тёмная", "Playing": "Запущена", "Available": "Доступна", "Source": "Источник", "Version": "Версия", "Loader": "Загрузчик", "Review": "Проверка", "Install": "Установка", "Choose a source": "Выберите источник", "Use existing package metadata": "Использовать метаданные пакета", "Search projects and versions": "Искать проекты и версии", "Search and install mods through the existing Modrinth, CurseForge and local resource workflows.": "Ищите и устанавливайте моды через Modrinth, CurseForge и локальные ресурсы.", "Cloudy Launcher": "Cloudy Launcher"
    };
    const walker = document.createTreeWalker(workspace, NodeFilter.SHOW_TEXT);
    const nodes = [];
    while (walker.nextNode()) nodes.push(walker.currentNode);
    nodes.forEach((node) => {
      const trimmed = node.nodeValue.trim();
      if (translations[trimmed]) node.nodeValue = node.nodeValue.replace(trimmed, translations[trimmed]);
    });
    const search = document.getElementById("instance-search");
    if (search) search.placeholder = "Поиск сборок";
  };

  function render() {
    const currentState = state();
    applyWeatherTheme(currentState.weatherTheme);
    applyPalette(currentState.palette);
    if (route === "new") renderNew(currentState);
    else if (route === "mods") renderMods(currentState);
    else if (route === "files") renderFiles(currentState);
    else if (route === "instance") renderInstance(currentState);
    else if (route === "accounts") renderAccounts(currentState);
    else if (route === "settings") renderSettings(currentState);
    else if (route === "skins") renderSkins(currentState);
    else renderHome(currentState);

    const activeAccount = currentState.accounts.find((item) => item.active) || currentState.accounts[0];
    const runningItems = currentState.instances.filter((item) => item.running);
    const running = runningItems.length;
    const runningLabel = running === 0
      ? copy("No instances are currently running", "Сейчас нет запущенных сборок")
      : running === 1
        ? `${copy("Running", "Запущена")}: ${runningItems[0].name}`
        : `${running} ${copy("instances are currently running", "сборки сейчас запущены")}`;
    topbarRunningText.textContent = runningLabel;
    topbarRunningCard?.classList.toggle("has-running", running > 0);
    if (topbarRunningCard) topbarRunningCard.dataset.instanceId = runningItems[0]?.id || "";
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
    translateVisibleUi();
  }

  document.addEventListener("click", (event) => {
    const onboardingButton = event.target.closest("[data-onboard-action]");
    if (onboardingButton) {
      const action = onboardingButton.dataset.onboardAction;
      const value = onboardingButton.dataset.value || "";
      if (action === "language") {
        try { localStorage.setItem("cloudy-language", value); } catch (_) {}
        document.documentElement.lang = value === "ru" ? "ru" : "en";
        callNative("setLanguage", value === "ru" ? "ru_RU" : "en_US");
        render();
      } else if (action === "style") {
        chooseWeatherTheme(value);
      } else if (action === "microsoft") {
        onboardingAuth = "microsoft";
        callNative("openAccounts");
        render();
      } else if (action === "nickname") {
        onboardingAuth = "nickname";
        render();
      } else if (action === "create-nickname") {
        const input = document.getElementById("onboard-nickname");
        onboardingNickname = input?.value.trim() || "";
        if (callNative("addOfflineAccount", onboardingNickname) !== false) {
          onboardingAuth = "";
          render();
        }
      } else if (action === "back") {
        saveOnboardingStep(onboardingStep - 1);
        render();
      } else if (action === "next") {
        if (onboardingStep === 3) {
          const memory = Number(document.getElementById("onboard-memory")?.value || 4096);
          callNative("setGlobalMemory", memory);
        }
        if (onboardingStep >= 5) {
          callNative("completeFirstRun");
          try { localStorage.removeItem(onboardingStorageKey); } catch (_) {}
          onboardingStep = 1;
          setRoute("home");
        } else {
          saveOnboardingStep(onboardingStep + 1);
          render();
        }
      }
      event.stopPropagation();
      return;
    }

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
      callNative("setSnowVariant", snowButton.dataset.snowVariant);
      render();
      event.stopPropagation();
      return;
    }

    const actionButton = event.target.closest("[data-action]");
    if (actionButton) {
      const action = actionButton.dataset.action;
      const card = actionButton.closest("[data-instance-id]");
      const id = actionButton.dataset.instanceId || card?.dataset.instanceId || state().instances[0]?.id || "";
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
      if (action === "open-running" && id) { selectedInstanceOverride = id; callNative("selectInstance", id); setRoute("instance"); }
      if (action === "open-instance" && id) { selectedInstanceOverride = id; callNative("selectInstance", id); setRoute("instance"); }
      if (action === "open-native-instance" && id) { callNative("openInstancePage", id, "overview"); }
      if (action === "toggle-sounds") { callNative("setSoundsEnabled", !state().soundsEnabled); }
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
      selectedInstanceOverride = card.dataset.instanceId;
      callNative("selectInstance", card.dataset.instanceId);
      setRoute("instance");
    }
  });

  const topbar = document.querySelector(".topbar");
  topbar?.addEventListener("pointerdown", (event) => {
    if (event.button === 0 && !event.target.closest("button")) callNative("beginWindowDrag");
  });

  document.addEventListener("input", (event) => {
    if (event.target.id === "onboard-memory") {
      const value = Number(event.target.value || 4096);
      const valueLabel = document.querySelector(".memory-value strong");
      if (valueLabel) valueLabel.textContent = `${value} MB`;
      callNative("setGlobalMemory", value);
      return;
    }
    if (event.target.id !== "instance-search") return;
    const query = event.target.value.trim().toLowerCase();
    document.querySelectorAll(".instance-card").forEach((card) => {
      card.hidden = query && !card.textContent.toLowerCase().includes(query);
    });
  });

  navButtons.forEach((button) => button.addEventListener("keydown", (event) => {
    if (event.key === "Enter" || event.key === " ") button.click();
  }));

  document.documentElement.lang = uiLanguage();
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
