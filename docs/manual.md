## BI-ARD Semestrální práce – Smart sensor teploty a vlhkosti
# HW dokumentace
Zapojení jsem už popsal v teoretické případě a nic se v něm nemění. Závisí, jaký způsob ukládání dat zvolíte a podle toho je potřeba mít součástky. Je teoreticky možné použít více senzorů než jeden, ale kód by musel být upraven, aby s tím dokázal pracovat správně. 

Pro mě to bylo primárně: (verze Arduino)

- PIN2 – teplotní čidlo DHT22
- PIN4 – SPI CS pro čtečku SD karet
- PIN10 – W5100 Ethernet
# Manuál
Jak už bylo zmínění v kódu je potřeba definovat několik věcí, včetně zvoleného způsobu ukládání dat a použitých pinů. Mimo toho také (podle použití) MAC adresu pro Ethernet, MQTT server a topicy, NTP Server a časovou zónu, frekvenci logování dat v ms.

![](img/m/9.png)

![](img/m/10.png)

MQTT nebo SD kartu vybereme odkomentováním jednoho a zakomentováním toho co nepoužijeme. V kódu jsou podmíněné bloky, které ho umožňují zkompilovat podle naší volby. Většinu chyb jsem se pokusil, pokud možno přímočaře okomentovat v debug zprávách, co by se vám měly zobrazit v sériové konzoli při připojení k PC.
## SD karta
Pokud používáme SD kartu, je nutné ji nejdříve zformátovat ve formátu FAT16/FAT32. Je taky potřeba aby karta byla maximálně typu SDHC, tedy karty větší, jak 32 GB nebudou fungovat. Potom musíme nastavit pin, kde je SPI rozhraní (chip select) SD karty připojeno. Dále je potřeba nastavit NTP server (například pool.ntp.org nebo time.nist.gov) a offset od naší časové zóny oproti GMT (v zimě 1 h, v létě 2 h => 3600/7200). 

Pokud všechno funguje, stačí navštívit adresu Arduina (pokud nevíte, najdete jít v debug logu seriové konzole po spuštění programu), například <http://192.168.1.10> a měli bychom vidět naší aktuální teplotu a vlhkost! Na vaší SD kartu by se měla ukládat všechna data ve formátu CSV, rozdělená po jednotlivých souborech v případě resetu zařízení.

![](img/m/11.png)
## MQTT & Home Assistant
Zde je konfigurace trochu složitější, ale pokusím se to shrnout co nejjednodušeji. Pro instalaci HomeAssistenta vás musím odkázat na jeho dokumentaci, liší se podle zařízení: <https://www.home-assistant.io/installation/>. Ale já doporučuju Home Assistant OS na Raspberry Pi nebo nějakém vašem počítači. Po spuštění a instalaci projdete prvním setup, kde je potřeba vytvořit účet. Uživatelské jméno a heslo si zapamatujte, bude potřeba pro připojení k MQTT. 

Dalším krokem je přidání MQTT addonu a jeho nastavení. V levé části obrazovky klikneme na nastavení, a potom Doplňky (Addons). Potom klikneme na obchod s doplňky a hned mezi prvními by mělo být „Mosquitto broker“, na to klikneme a nainstalujeme.

![](img/m/12.png) 

![](img/m/13.png)

![](img/m/14.png)

![](img/m/15.png)

Potom je potřeba doplňek spustit. Dalé jděte do Nastavení – Zařízení a služby – Přidat integraci a vyberte MQTT. Tady by mělo stačit proklikat výchozí nastavení.

![](img/m/16.png)

A to je vše pro MQTT! Posledním krokem je upravení konfigurace v YAML. Můžete otevřít konfigurační soubor configuration.yaml různými způsoby, ale asi nejlepší možnost je další doplněk – File editor. Ten potom stačí otevřít, najít správný soubor a potom už je upravit několik řádků konfigurace.

![](img/m/17.png)

Pro základní konfiguraci můžete použít tento můj kód: 

![](img/m/18.png)

Posledním krokem je přidání topiců, které jsme si výše definovali do našeho kódu. To uděláme ve vyznačené sekci na obrázku níže. Zde je potřeba použít stejné jako v HA. IP adresa bude IP adresa našeho serveru a username a password bude stejný jako váš účet v Home Assistantovi. 

` `![](img/m/19.png)

Pokud se vám to povedlo, měla by komunikace nyní probíhat. Také byste měli v Nastavení – Zařízení a služby – Zařízení (úplně nahoře) vidět váš sensor. Když ho vyberete měli byste se dostat na obrazovku kde budou všechny jeho data (viz obrázek níže). Pokud chcete, můžete si ho rovnou přidat do ovládacího panelu tlačítkem „Přidat do ovládacího panelu“.

![](img/m/20.png)

*(Toto je příklad jiného sensoru, rozhraní ale vypadá přibližně stejně.)*
## ESP8266 verze
Rozdíly ve verzi pro ESP8266 už nebudu v návodu rozepisovat, největší je asi zadání názvu wifi sítě a hesla podobně jako jsme zadávali například piny v kódu. Mělo by být možné používat jak SD kartu, tak MQTT, ale neměl jsem možnost to podrobně otestovat.

