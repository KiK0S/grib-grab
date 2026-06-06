const SHROOMS_BUILD_VERSION = "32eae7fc95f94f84d8970377b4899827ce708bea";
const SHROOMS_RUNTIME_CACHE = `shrooms-runtime-${SHROOMS_BUILD_VERSION}`;
const SHROOMS_RUNTIME_ASSETS = [
  "shrooms.js",
  "shrooms.wasm",
];
const SHROOMS_RUNTIME_BASENAMES = new Set(SHROOMS_RUNTIME_ASSETS);

function requestBasename(request) {
  const url = new URL(request.url);
  const parts = url.pathname.split("/");
  return parts[parts.length - 1] || "";
}

function runtimeAssetRequest(name, cacheMode = "default") {
  const url = new URL(name, self.registration.scope).href;
  if (cacheMode === "default") {
    return new Request(url);
  }
  return new Request(url, { cache: cacheMode });
}

async function populateRuntimeCache() {
  const cache = await caches.open(SHROOMS_RUNTIME_CACHE);
  const requests = SHROOMS_RUNTIME_ASSETS.map((name) => runtimeAssetRequest(name, "reload"));
  const responses = await Promise.all(requests.map((request) => fetch(request)));
  if (!responses.every((response) => response && response.ok)) return;
  await Promise.all(
      responses.map((response, index) => cache.put(requests[index], response.clone())),
  );
}

async function cacheFirst(request) {
  const cache = await caches.open(SHROOMS_RUNTIME_CACHE);
  const cached = await cache.match(request);
  if (cached) return cached;
  const response = await fetch(request);
  if (response && response.ok) {
    await cache.put(request, response.clone());
  }
  return response;
}

self.addEventListener("install", (event) => {
  event.waitUntil(populateRuntimeCache());
});

self.addEventListener("activate", (event) => {
  event.waitUntil(
      caches.keys().then((keys) =>
          Promise.all(
              keys
                  .filter((key) => key.startsWith("shrooms-runtime-") &&
                      key !== SHROOMS_RUNTIME_CACHE)
                  .map((key) => caches.delete(key)),
          )),
  );
});

self.addEventListener("fetch", (event) => {
  if (event.request.method !== "GET") return;
  if (!SHROOMS_RUNTIME_BASENAMES.has(requestBasename(event.request))) return;
  event.respondWith(cacheFirst(event.request));
});
