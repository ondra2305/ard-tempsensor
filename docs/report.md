## BI-ARD Semestrální práce – Smart sensor teploty a vlhkosti
# Zpráva
Semestrální práce na Arduino mě bavila, a nakonec jsem se dostal k funkčnímu řešení, s kterým jsem relativně spokojený. To ale neznamená, že jsem nenarazil na žádné zádrhele. Jak už jsem předpověděl v teoretické přípravě, narazil jsem na problém, že Arduino Uno prostě není na toto moje použití vhodné, a to primárně z důvodu nedostatku paměti a úložiště (pouze 2KB + 32KB).

` `Vzhledem k tomu, že jsem chtěl provozovat web sever, už ze začátku jsem potřeboval docela dost knihoven, abych to celé mohl zprovoznil. Chtěl jsem zároveň integrovat možnost logování na SD kartu a připojení k MQTT serveru. To se mi nakonec nepovedlo. Pro hostování webserveru jsem potřeboval mít v kódu hardcoded aspoň nějaké základní HTML, a jak jsem zjistil stringy zabírají nejvíce paměti RAM. Někdy se mi i povedlo program zkompilovat, ale narazil jsem na korupci dat nebo crashe.

![](img/m/1.png)

Řešení, které jsem nakonec zvolil, je nechat uživatele prostřednictvím definů v kódu zvolit, jestli chce používat logování na SD kartu, nebo přes MQTT do Home Assistenta. Obě dvě verze fungují, přičemž ve funkčnosti se mění pouze způsob ukládání dat. 
## Logování na SD kartu
Pokud zvolíme možnost logování na SD kartu, je nutné zvolit interval logování. (Funguje to i s 1s, každopádně u Arduina Una nedoporučuju, spíše tak 10s.) Arduino tak například každých 10s uloží data ze senzoru do souboru SDATA.CSV. Při resetu anebo při dosažení určitého počtu záznamů je soubor přejmenován na OLDXXXXX.CSV a zapisuje se nanečisto. Data v souboru jsou ve formátu:

UNIXTIME,TEMP,HUM. Každý záznam je na vlastním řádku. To by mělo relativně usnadnit práci s daty.

![](img/m/2.png)
## MQTT+Home Assistent
Tahle verze je trochu zajímavější, alespoň podle mého názoru. Pro testování jsem si stáhl Home Assistant OS, který obsahuje všechno potřebné ve verzi VM pro VirtualBox. Po nějaké základní konfiguraci jsem přidal addon na MQTT. Tento MQTT server se poté zadá do našeho kódu a Arduino umí odesílat data na MQTT, kde si je HomeAssistant převezme. S jednoduchou konfigurací v YAML můžeme přidat náš senzor (viz obrázek). V podstatě jen definujeme topicy, kam bude sensor posílat data. Ty musíme také samozřejmě definovat v našem kódu. Výsledkem je funkční entita v Home Assistentovi, kterou můžeme použít například na další automatizace.
![](img/m/3.png)

![](img/m/4.png)
## ESP8266
Protože mě to zajímalo a myslím, že to je celkem zajímavé i v kontextu této semestrální práce, upravil jsem ještě můj kód tak, aby mohl fungovat s ESP8266 jako hlavním procesorem místo Arduina Una. Tady jsem samozřejmě mohl použít knihovny na logování na SD kartu zároveň s odesíláním dat na MQTT, místo problém nebylo. Dokonce mi to i umožnilo udělat o něco hezčí UI pro mojí HTML stránku.

Problémem ale bylo, že můj shield kombinující Ethernet a SD kartu, který jsem doposud používal s Arduinem, není kompatibilní s mojí ESP deskou kvůli rozdílnému napětí. (Asi? Nejsem si upřímně moc jistý, ale tohle je nepravděpodobnější verze. Každopádně jsem to nikdy nezprovoznil.) Na to by byly potřeba další součástky, které jsem ale v době dělání hlavní částí práce neměl a díky ne příliš rychlému doručení lokálních e-shopů jsem je ani nestihl koupit a vyzkoušet. Každopádně jsem otestoval moje ESP minimálně s MQTT a senzorem a všechno fungovalo skvěle. Na webserver jsem i přidal http přihlášení a použil jsem tam nějakou JS knihovnu na vytvoření grafů z dat na SD kartě.

![](img/m/5.png) 

![](img/m/6.png)
## Ukázka čidla v Home Assistantovi
![](img/m/7.png)
![](img/m/8.png)

*(Graf není pěkný kvůli tomu, že jsem to nenechal běžet dost dlouho a často jsem to restartoval…)*
## Závěr
Myslím, že se mi povedlo vytvořit relativně funkční verzi DIY domácího teplotního a vlhkostního senzoru. Pokud bych měl 3D tiskárnu, tak by byl dobrý nápad třeba vytisknout nějakou krabičku a udělat z toho trochu více permanentní řešení. Po tom, co jsem to mohl vyzkoušet, silně doporučuju na podobné projekty používat ESP8266/ESP32 a pokud bych to dělal od začátku taky, narazil bych na daleko méně problémů. Je lepší projít si tímhle vším než koupit nějaké smart-home teplotní čidlo? Spíše ne, ale i tak mě tento projekt bavil a kód co jsem vytvořil umožňuje relativně dobře porovnat vlastnosti Arduina a ESP.