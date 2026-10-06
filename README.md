# WXClient

A C++ client for the ICT Protege WX access controller, using its HTTP API.

## Status

The CLI frontend is working.

- **Doors, Areas, Outputs, Inputs, Trouble Inputs**: list records with decoded live status. Selecting
  a door, area, output or input opens its control commands; Trouble Inputs are view-only.
- **Event Logs**: browse events 20 at a time (Latest / Previous / Next), or download a date range as CSV.
- **Backup**: download the programming database as a `.bak` file.
- **Restart Modules / Restart Controller**: both ask for confirmation first.

Type `exit` at any menu to log out. Downloads are saved to `~/Downloads` as
`Db_<serial>_<dd_Mon_yyyy>.bak` and `Events_<serial>_<dd_Mon_yyyy>.csv`. A second download on the
same day overwrites the first.

The GUI frontend is a stub.

## Requirements

| Dependency     | Version    | Notes                                                               |
|----------------|------------|---------------------------------------------------------------------|
| CMake          | 3.22+      | tested up to 4.4                                                    |
| C++ compiler   | C++20      | `std::ranges`, `std::span`, `starts_with`, `std::from_chars`        |
| OpenSSL        | any recent | via `find_package(OpenSSL REQUIRED)`                                |
| cpp-httplib    | 0.53.1     | vendored at `include/vendor/httplib/httplib.h`                      |
| Linux          | —          | `termios`, `$HOME`, `localtime_r`; see [Roadmap](#roadmap)          |
| Network path   | direct     | reverse proxy not yet supported; see [Network path](#network-path)  |

`CPPHTTPLIB_OPENSSL_SUPPORT` is set in `CMakeLists.txt`; without it httplib
rejects `https://` URLs at runtime rather than at compile time.

## Building and running

### CLI (built by default)

```sh
cmake -B build
cmake --build build
./build/WXClient_CLI
```

Prompts for IP/domain, username, password, and whether the controller (not your connection to it) uses
HTTPS; see [Network path](#network-path) for why that distinction matters.

### GUI (stub)

```sh
cmake -B build -DWXCLIENT_BUILD_GUI=ON
cmake --build build
```

Add `-DWXCLIENT_BUILD_CLI=OFF` to build only the GUI.

## Network path

The client must reach the controller directly — reverse proxy support
is planned for the GUI only. `isHttps` currently answers two questions
at once: which scheme the client connects with, and which session mode the
controller is in (HTTP → `CheckPassword` + AES-encrypted parameters; HTTPS →
`CheckPasswordServer` + plain text). A gateway that terminates TLS puts those
two hops in different modes.

## Authentication

Two firmware flows, selected automatically:

- **4.00.1676+** — session identified solely by the `Set-Cookie` cookie.
- **Earlier** — `InitSession` fails, so the client retries with its own
  random session ID, appended to `InitSession`/`CheckPassword` and prepended
  to every later request once logged in.

Both start with `InitSession` returning a random number, XORed against the
username/password-hash and SHA-1'd. Then:

- **HTTP** — `CheckPassword`; a second random number seeds an AES-128 key
  (first 16 chars of a SHA-1). Parameters travel as `hex(iv) || hex(ciphertext)`.
- **HTTPS** — `CheckPasswordServer`; TLS provides confidentiality, no key is
  derived.

`CloseSession` runs from `ControllerApi`'s destructor, so the session closes
on every exit path, including exceptions.

## Security notes

- Controllers ship self-signed certificates, so HTTPS uses trust on first use, like SSH
  `known_hosts`. The first login shows the certificate's SHA-256 fingerprint and asks
  whether to trust it; accepted fingerprints are saved per host in `known_certificates` (see
  [Troubleshooting](#troubleshooting)). A changed certificate gets a louder warning: a
  renewal or factory reset causes it, and so would an active MITM. Compare the fingerprint
  with the controller's own (e.g. Firefox's certificate viewer, or
  `openssl s_client -connect <IP>:443 </dev/null | openssl x509 -noout -fingerprint -sha256`).
- Over HTTP the AES key is 16 hex *characters*, not decoded bytes — roughly
  64 bits of entropy in a 128-bit key. That's the firmware's design, not
  this client's.
- Passwords are never echoed or logged; error messages carry controller
  responses only, never request parameters.

## Layout

`wxclient_core` (a CMake `OBJECT` library) holds everything frontend-agnostic;
each frontend links against it as its own executable, so a GUI never pulls in
`console.cpp`'s terminal code.

| Core file            | Role                                                    |
|----------------------|---------------------------------------------------------|
| `controller_api`     | session, encryption, requests and commands              |
| `workflow`           | login + settings, cached record lists, statuses, events |
| `file_transfer`      | backup and event log downloads saved to disk            |
| `known_certificates` | trusted certificate fingerprints, one per host          |
| `status`, `control`  | decode status strings and look up control commands      |
| `lookup/`            | `constexpr` tables transcribed from the vendor docs     |

Menu keys and the top-level menu stay in the CLI's `session.cpp`.

## Troubleshooting

State lives in `$XDG_STATE_HOME/wxclient/` (default `~/.local/state/wxclient/`):

- `logs.csv`: failures are appended here; the CLI prints this path with every error.
  After any failed `ControllerApi` call, `wx.lastError()` holds the same message.
- `known_certificates`: trusted HTTPS certificates, one `host fingerprint` per line. Delete a
  line to be asked about that controller's certificate again.

| Response                               | Meaning                                                       |
|----------------------------------------|---------------------------------------------------------------|
| `FAIL`                                 | wrong username or password                                    |
| `FAIL 5` / `FAIL 60`                   | wrong credentials, repeated attempts — wait that many seconds |
| `FAIL. No valid operator login found.` | controller still on default `admin:admin`                     |

| Message                                         | Cause                                                      |
|-------------------------------------------------|------------------------------------------------------------|
| `Could not reach controller: Connection`        | wrong `Https?` answer, or nothing listening on that port   |
| `Could not reach controller: SSLConnection`     | answered HTTPS, TLS handshake failed                       |
| `Could not reach controller: Read`/`Write`      | connected, then timed out (5 s requests, 300 s downloads)  |
| `Controller returned HTTP <status>`             | transport fine, request form rejected                      |
| `Controller returned HTTP 301`/`302`            | redirect, not followed — usually a middlebox forcing HTTPS |
| `Failed to finalize decryption`                 | session key mismatch                                       |
| `Command Failed (128) Invalid Command SubType.` | the controller doesn't accept Control for that table       |

Isolate transport from protocol with a raw `InitSession` call:

```sh
curl -vk "<http|https>://<IP>/PRT_CTRL_DIN_ISAPI.dll?Command&Type=Session&SubType=InitSession"
```

A bare number in the body means transport and request form are correct.

## Roadmap

- GUI frontend: toolkit not chosen. Will add what the CLI deliberately leaves out:
  - Submit/Delete for records.
  - Reverse proxy support. Core needs `isHttps` split into a client transport flag
    and a controller session-scheme flag (see Network path); the CLI keeps one prompt and sets both.
- Uploads: CSV imports (e.g. user lists) and restoring `.bak` backups, in `file_transfer`.
- Hardware verification: the pre-4.00.1676 flow is implemented but untested.
- Windows: `console.cpp` is POSIX-only by design; a GUI would share only
  `core/`, where `stateDirectory()` (helpers) and `defaultDownloadDirectory` need Windows paths.