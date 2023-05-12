## BI-ARD Semestrální práce – Sensor teploty a vlhkosti
# Specifikace zadání
Můj cíl je udělat DIY senzor teploty/vlhkosti s připojením na internet (Wifi nebo příp. Ethernet) s data loggingem stavů na SD kartu. Po zadání adresy senzoru do browseru by mělo Arduino hostovat jednoduchý web server, kde by se měla zobrazit aktuální teplota/vlhkost a graf například stavu za posledních 24 h. Ještě by stálo za to implementovat třeba upozornění při překročení nějaké hranice teploty nebo vlhkosti třeba přes Discord notifikace nebo něco podobného.
# Teoretická příprava
Inspirací zde jsou komerční sensory teploty, vlhkosti, a mě zajímá, jestli se mi povede něco podobného vytvořit jen za pomocí Arduina. Rád bych výsledek doma použil, ale bude to spíš takový “proof of concept”. Celý projekt si plánuji sestavit z vlastních součástek.

## Součástky a hardware
Pro tento projekt budu pravděpodobně potřebovat některé z následujících součástek:

- Arduino deska (např. Arduino Uno nebo Arduino Mega)

Arduino Uno nebo deska s integrovaným ESP <https://www.laskakit.cz/wemos-d1-r2-uno-esp8266/> nebo modul ESP 8266 (tady by byl potřeba převodník logických úrovní)[](https://www.laskakit.cz/wemos-d1-r2-uno-esp8266/)

- Senzor teploty a vlhkosti DHT22 (nebo DHT11/podobný)

<https://www.laskakit.cz/arduino-senzor-teploty-a-vlhkosti-vzduchu-dht22--modul/>

- Modul Ethernet (např. ESP8266 nebo W5100)

<https://www.laskakit.cz/arduino-ethernet-shield-w5100/>

- MicroSD karta pro ukládání dat

Součástí Ethernet shieldu nebo <https://www.laskakit.cz/tzt-data-logger-shield-v1-0/>

- Displej pro zobrazování aktuální teploty a vlhkosti (volitelné)

Například: <https://www.laskakit.cz/arduino-1602-lcd-klavesnice-shield/>

Displej asi nakonec nepoužiji, ale bude se určitě hodit na debuggování

Prvním krokem je připojení senzoru teploty a vlhkosti k Arduino desce. Pin senzoru DHT22 pro přenos dat se připojí k digitálnímu pinu desky (např. pinu 2), a kromě toho se připojí společný pin pro napájení (VCC) a pin pro zem (GND).

Dalším krokem je připojení modulu Wifi nebo Ethernet k desce. Pokud používáme modul Wifi, pak se připojí k digitálním pinům (např. pinům 10, 11, 12, 13 na desce Arduino Uno) pro komunikaci přes sériovou linku. Pokud používáme modul Ethernet, pak se připojí k desce pomocí Ethernetového modulu, který se připojí k desce pomocí SPI rozhraní a připojí se k Ethernetovému kabelu. Předpokládám, že finální rozhodnutí, jestli využiji Wifi nebo Ethernet (nebo snad oboje) padne v průběhu kódování programu.

Poté můžeme začít s programováním. Nejprve je nutné nainstalovat knihovny pro senzor teploty a vlhkosti, modul Wifi nebo Ethernet a knihovny pro zápis na MicroSD kartu. Je potřeba použít knihovny SPI, SD a DHT anebo alespoň jejich část.

Následně můžeme začít s vytvořením kódu pro Arduino. Budeme potřebovat vytvořit funkce pro měření teploty a vlhkosti, posílání dat na server, ukládání dat na MicroSD kartu a hostování webového serveru. Chci udělat aktualizace teploty na webu automatické, že například každých 5 s by se stránka obnovila (možnost dát do do loop funkce Arduina).

Tady mám v plánu použít Arduino MQTT knihovnu <https://github.com/256dpi/arduino-mqtt> a/nebo knihovnu na Discord webhooky: <https://github.com/usini/usini_discord_webHook>.

Mým cílem by byla ale pokud všechno půjde dobře integrace se Smart Home systémem Home Assistentem přes MQTT server. MQTT server a Home Assistant bude pro mojí demonstraci pravděpodobně běžet na Raspberry Pi nebo mém notebooku, ale je samozřejmě možné použít jakýkoliv počítač nebo server.

Pro integraci s HA můžeme použít protokol MQTT. Ten má přímo integraci s MQTT serverem: <https://www.home-assistant.io/integrations/mqtt/>. Potřebujeme se připojit k serveru MQTT a publikovat data na určité téma, Home Assistant tohle téma bude poté číst a zobrazovat jako senzor v našem Smart Home dashboardu.

![MQTT Working, Types, Applications](img/t/1.png)

*Obrázek 1: Schéma komunikace MQTT, v našem případě bude subscriber Home Assistant a MQTT message broker nějaký náš server*

## Možné problémy:

Při vývoji a implementaci projektu může nastat několik potenciálních problémů:
### Nepřesné měření teploty a vlhkosti
- Je možné, že narazím na problém s nepřesností senzorů, i když doufám že ne. Mám v plánu jeho přesnost srovnat s jinými senzory teploty a vlhkosti a v případě větší odchylky by neměl být problém sensor DHT22 v kódu mírně zkalibrovat.
### Nestabilita spojení s Wi-Fi nebo Ethernet
- Teoreticky by mohly nastat problémy se slabým signálem u ESP8266 modulu nebo celkově s výpadky připojení
### Omezený výkon Arduino
- Arduino má omezený výkon a paměť, což může omezit schopnost zařízení zpracovávat a ukládat velké množství dat. Bude potřeba programovat efektivně. Největší problém tady bude asi v případě použití Arduina Uno jeho velikost RAM (která je 2 kB), což bude problém v případě použití více knihoven nebo při větší složitosti programu. Možná bude nutné části knihoven vyřadit nebo si je naprogramovat from scratch. 
- Mým zachráncem v nejhorším případě bude deska Wemos, kde zle použít jako CPU integrované ESP 8266, které má daleko více paměti (32 kB), to ale může zase způsobovat jiné problémy s kompatibilitou.

## Schéma zapojení
![Diagram, schematic
Description automatically generated](img/t/2.png)

*Obrázek 2: Schéma zapojení verze 1 - Ethernet*

Na obrázku je možné vidět zapojení v případě Ethernetu. V mém případě bude ale pravděpodobně zapojování o dost méně protože použiji Ethernet shield s integrovanou SD kartou, ale vnitřní zapojení pinů bude velmi podobné. Ve webovém programu na tvorbu schématu nebylo možné zvolit Ethernet shield.

![Diagram, schematic
Description automatically generated](img/t/3.png)

*Obrázek 3: Schéma zapojení verze 2 – Wifi ESP*

Na druhém obrázku je vidět možné zapojení s ESP8266 Wifi modulem. Alternativou jde tady použití výše zmíněné Wemos D1 desky, která má modul ESP přímo na sobě. V tomto případě je v zapojení vidět i převodník logických úrovní, který je pro připojení ESP modulu přímo do Arduino Uno potřeba, protože logika modulu používá 3V3 místo 5V v případě Arduina. (<https://www.laskakit.cz/4-kanaly-obousmerny-prevodnik-logickych-urovni-5v-a-3-3v/>)
2
