// CYD (ESP32-2432S028) flashcards - CompTIA A+ + cybersecurity terms
// Libs: TFT_eSPI (configured for CYD), XPT2046_Touchscreen
// Tap center: flip term/def. Tap "<" / ">": prev/next card (resets to term side). Long-press: shuffle.

#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <SPI.h>

#define XPT2046_IRQ  36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK  25
#define XPT2046_CS   33

// Raw touch calibration. XPT2046 reports raw ADC values, not screen pixels.
// If arrows feel reversed/off, Serial.print(p.x, p.y) in loop() and adjust these.
#define TS_MINX 200
#define TS_MAXX 3700
#define TS_MINY 240
#define TS_MAXY 3800
#define SCREEN_W 320
#define SCREEN_H 240
#define ARROW_ZONE 60 // px from each edge counted as arrow tap

TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);

struct Card { const char* term; const char* def; };

Card deck[] = {
  {"[Security] CIA Triad", "Confidentiality, Integrity, Availability"},
  {"[Security] Phishing", "Social engineering via fraudulent messages to steal info"},
  {"[Security] Zero-Day", "Vuln unknown to vendor, no patch exists yet"},
  {"[Security] MFA", "Multi-Factor Auth: 2+ proof types (know/have/are)"},
  {"[Security] Hashing", "One-way transform, verify integrity, not reversible"},
  {"[Security] Salting", "Random data added to password before hash, stops rainbow tables"},
  {"[Security] Firewall", "Filters traffic by rules, network perimeter control"},
  {"[Security] IDS vs IPS", "IDS detects+alerts, IPS detects+blocks"},
  {"[Security] Least Privilege", "Grant minimum access needed to do the job"},
  {"[Security] CVE", "Common Vulnerabilities and Exposures, public vuln ID"},
  {"[Security] Man-in-the-Middle", "Attacker intercepts/relays comms between two parties"},
  {"[Security] Symmetric Encryption", "Same key encrypts and decrypts (fast, key distribution problem)"},
  {"[Security] Asymmetric Encryption", "Public/private keypair, solves key distribution"},
  {"[Security] TLS", "Transport Layer Security, encrypts data in transit"},
  {"[Security] DDoS", "Distributed Denial of Service, flood target from many sources"},
  {"[Security] SQLi", "SQL Injection, unsanitized input alters DB query"},
  {"[Security] XSS", "Cross-Site Scripting, inject script into web page viewed by others"},
  {"[Security] Privilege Escalation", "Gaining higher access than originally granted"},
  {"[Security] Honeypot", "Decoy system to lure/study attackers"},
  {"[Security] APT", "Advanced Persistent Threat, stealthy long-term targeted intrusion"},
  {"[Security] Social Engineering", "Manipulating people (not systems) to gain access/info"},
  {"[Security] Rootkit", "Malware hiding its presence, often kernel/boot-level access"},
  {"[Security] Ransomware", "Malware that encrypts data, demands payment for key"},
  {"[Networking] VPN", "Virtual Private Network, encrypted tunnel over untrusted network"},
  {"[Networking] Public vs Private IP", "Public routable on internet, private (RFC1918) internal only"},
  {"[Networking] IPv4 vs IPv6", "IPv4: 32-bit, ~4.3B addrs. IPv6: 128-bit, vastly larger space"},
  {"[Networking] DHCP", "Assigns IP config automatically to devices on a network"},
  {"[Networking] DNS", "Resolves domain names to IP addresses"},
  {"[Networking] NAT", "Network Address Translation, maps private IPs to public for internet access"},
  {"[Networking] Subnet Mask", "Defines network vs host portion of an IP address"},
  {"[Networking] OSI Model", "7 layers: Physical, Data Link, Network, Transport, Session, Presentation, App"},
  {"[Networking] TCP vs UDP", "TCP: reliable/connection-based. UDP: fast/connectionless, no guarantee"},
  {"[Hardware] RAM Types", "DDR3/DDR4/DDR5, higher gen = faster, not backward compatible w/ slots"},
  {"[Hardware] SSD vs HDD", "SSD: flash, fast, no moving parts. HDD: spinning platters, slower, cheaper/GB"},
  {"[Hardware] BIOS vs UEFI", "BIOS: legacy, MBR, 16-bit. UEFI: modern, GPT, faster boot, Secure Boot support"},
  {"[Hardware] POST", "Power-On Self-Test, hardware check on boot before OS loads"},
  {"[Hardware] CPU Socket", "Physical interface CPU plugs into on motherboard, must match chipset"},
  {"[Hardware] RAID 0 vs RAID 1", "RAID 0: striping, speed, no redundancy. RAID 1: mirroring, redundancy"},
  {"[Hardware] USB Versions", "USB 2.0 ~480Mbps, 3.0 ~5Gbps, higher = faster transfer speed"},
  {"[Hardware] Printer Types", "Laser: toner/heat, fast/cheap per page. Inkjet: liquid ink, better color detail"},
  {"Safe Mode", "Windows boots w/ minimal drivers/services, used for troubleshooting"},
  {"Windows Editions", "Home, Pro, Enterprise: differ in domain join, BitLocker, group policy support"},
  {"Task Manager", "View/kill running processes, monitor CPU/RAM/disk usage"},
  {"Device Manager", "View/manage installed hardware and drivers in Windows"},
  {"[Security] Malware Types", "Virus (needs host+user action), Worm (self-spreads), Trojan (disguised as legit)"},
  {"[Security] Patch Management", "Process of applying updates to fix vulns/bugs, keep systems current"},
  {"Data Backup Types", "Full: everything. Incremental: changes since last backup. Differential: changes since last full"},
  {"[Hardware] ESD", "Electrostatic Discharge, static damage risk to components, use wrist strap"},
  {"Mobile Device Sync", "Syncing contacts/email/files across devices via account/cloud"},
  {"[Networking] MAC Address", "Fixed hardware identifier on a device's network adapter, unlike IP it doesn't change"},
  {"[Networking] HTTP vs HTTPS", "HTTPS adds SSL/TLS encryption over HTTP, protects data in transit"},
  {"[Networking] Switch vs Router", "Switch connects devices within a LAN. Router connects LAN to outside network, handles IP addressing"},
  {"[Networking] Static vs Dynamic IP", "Static: fixed, used for servers/printers. Dynamic: changes, assigned via DHCP"},
  {"[Networking] Workgroup vs Domain", "Workgroup: peer-to-peer, local accounts. Domain: centrally managed by a domain controller"},
  {"[Hardware] Hard Reset vs Soft Reset", "Soft: restart via software, no power loss. Hard: full power cut, used when frozen"},
  {"Cloud Computing", "Storing data/running apps on remote servers accessed via internet, not local machine"},
  {"Active Directory (AD)", "On-prem directory service managing users, computers, and Group Policy in a domain"},
  {"Entra ID", "Microsoft's cloud identity platform, handles auth/SSO for M365, often synced with on-prem AD"},
  {"[Intune] What Is It", "Microsoft's cloud MDM/MAM tool for managing and securing endpoints and mobile devices"},
  {"[Security] Microsoft Defender", "Endpoint security suite: threat detection, antivirus, compliance monitoring"},
  {"SLA", "Service Level Agreement, target time/quality commitment for resolving a ticket"},
  {"Tier 1 vs Tier 2 Support", "Tier 1: initial triage, common issues. Tier 2: deeper troubleshooting, escalated/complex"},
  {"Onboarding/Offboarding", "Provisioning vs de-provisioning of accounts, access, and hardware for employees"},
  {"[Networking] APIPA", "Address like 169.254.x.x means DHCP failed to assign an IP, not that the network itself is down"},
  {"[Security] BitLocker", "Windows built-in disk encryption, protects data if a device is lost or stolen"},
  {"[Hardware] Print Spooler", "Windows service managing the print queue, restarting it often fixes stuck print jobs"},
  {"[Intune] Windows Autopilot", "Zero-touch device provisioning: new device auto-joins Entra ID and pulls Intune policy on first boot"},
  {"[Intune] MDM vs MAM", "MDM manages the whole device (Intune). MAM manages just work apps/data, common on personal (BYOD) phones"},
  {"[Intune] Compliance Policy", "Intune rule set (min OS version, encryption, passcode) a device must meet, non-compliant = flagged/restricted"},
  {"[Intune] Conditional Access", "Policy that grants/blocks access to resources (email, Teams) based on device compliance, location, or risk signals"},
  {"[Intune] Remote Wipe", "Intune/MDM action to erase a lost, stolen, or offboarded device's data remotely"},
};
const int deckSize = sizeof(deck) / sizeof(deck[0]);

int order[deckSize];
int pos = 0;
bool showingDef = false;
unsigned long touchStart = 0;
bool touching = false;
int touchX = 0;

void shuffleDeck() {
  for (int i = 0; i < deckSize; i++) order[i] = i;
  for (int i = deckSize - 1; i > 0; i--) {
    int j = random(i + 1);
    int t = order[i]; order[i] = order[j]; order[j] = t;
  }
  pos = 0;
}

// splits a "[Tag] Term" string into tag ("Tag") and the remaining term text.
// tag is empty if the term has no [Tag] prefix.
void splitTag(const char* term, String &tag, String &rest) {
  String s(term);
  if (s.startsWith("[")) {
    int endIdx = s.indexOf(']');
    if (endIdx != -1) {
      tag = s.substring(1, endIdx);
      rest = s.substring(endIdx + 2); // skip "] "
      return;
    }
  }
  tag = "";
  rest = s;
}

void drawCard() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  Card &c = deck[order[pos]];
  tft.setTextColor(showingDef ? TFT_GREEN : TFT_WHITE);
  tft.setTextFont(2);
  tft.drawString(showingDef ? "DEFINITION" : "TERM", 160, 20, 2);

  String tag, mainTerm;
  splitTag(c.term, tag, mainTerm);

  if (!showingDef && tag.length() > 0) {
    tft.setTextColor(TFT_CYAN);
    tft.setTextFont(1);
    tft.drawString(tag, 160, 38, 1);
  }

  tft.setTextColor(TFT_YELLOW);
  const char* text = showingDef ? c.def : mainTerm.c_str();
  // terms are short (fit big font), defs run long (need smaller font to avoid overflow)
  uint8_t bodyFont = showingDef ? 2 : 4;
  int lineHeight = showingDef ? 22 : 30;
  wrapAndDraw(text, 160, 120, 260, bodyFont, lineHeight);
  tft.setTextFont(1);
  tft.setTextColor(TFT_DARKGREY);
  char buf[16];
  sprintf(buf, "%d / %d", pos + 1, deckSize);
  tft.drawString(buf, 160, 220, 1);

  tft.setTextColor(TFT_CYAN);
  tft.setTextFont(4);
  tft.drawString("<", 25, 120, 4);
  tft.drawString(">", 295, 120, 4);
}

// word-wrap since TFT_eSPI has no built-in wrap. Vertically centers the block
// around cy so it can't run into the footer counter or side arrows regardless
// of how many lines a long definition needs.
#define MAX_WRAP_LINES 8
void wrapAndDraw(const char* text, int cx, int cy, int maxWidth, uint8_t font, int lineHeight) {
  tft.setTextFont(font);
  String s(text);
  static String lines[MAX_WRAP_LINES];
  int lineCount = 0;
  String line = "";
  int idx = 0;
  int n = s.length();
  while (idx <= n && lineCount < MAX_WRAP_LINES) {
    int spacePos = s.indexOf(' ', idx);
    if (spacePos == -1) spacePos = n;
    String word = s.substring(idx, spacePos);
    String test = line.length() ? (line + " " + word) : word;
    if (tft.textWidth(test) > maxWidth && line.length() > 0) {
      lines[lineCount++] = line;
      line = word;
    } else {
      line = test;
    }
    idx = spacePos + 1;
    if (spacePos == n) break;
  }
  if (line.length() && lineCount < MAX_WRAP_LINES) lines[lineCount++] = line;

  int y = cy - (lineCount * lineHeight) / 2 + lineHeight / 2;
  for (int i = 0; i < lineCount; i++) {
    tft.drawString(lines[i], cx, y, font);
    y += lineHeight;
  }
}

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(1);
  Serial.printf("tft size: %d x %d\n", tft.width(), tft.height());
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);
  randomSeed(esp_random());
  shuffleDeck();
  drawCard();
}

void loop() {
  bool isTouched = ts.touched();

  if (isTouched && !touching) {
    touching = true;
    touchStart = millis();
    TS_Point p = ts.getPoint();
    touchX = map(p.x, TS_MINX, TS_MAXX, 0, SCREEN_W);
  }

  if (!isTouched && touching) {
    unsigned long held = millis() - touchStart;
    touching = false;
    if (held >= 800) {
      shuffleDeck();
      showingDef = false;
    } else if (touchX < ARROW_ZONE) {
      pos = (pos - 1 + deckSize) % deckSize;
      showingDef = false;
    } else if (touchX > SCREEN_W - ARROW_ZONE) {
      pos = (pos + 1) % deckSize;
      showingDef = false;
    } else {
      showingDef = !showingDef;
    }
    drawCard();
  }

  delay(20);
}
