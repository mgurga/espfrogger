#include <Arduino.h>
#include <TFT_eSPI.h>
#include <Button2.h>
#include <limits.h>

#define ROAD_LENGTH 30
#define ROADS_ON_SCREEN 8

struct Car
{
    int x;
    int color;
    bool moveright; // otherwise move left
    int width;
    int speed;
};

TFT_eSPI tft = TFT_eSPI();
int roads[2000];
Car cars[2000];
int roadlength = 0;
int score = 0;
int scroll = 0;
int frame = 0;
int lastupdateframe = 0;
bool alive = true;

Button2 btn1(35);
Button2 btn2(0);

void drawScore()
{
    String out = String(score);
    tft.setTextSize(2);
    tft.drawString(out, 0, 0);
}

void drawRoad()
{
    for (int i = 0; i < ROADS_ON_SCREEN; i++)
    {
        int tftheight = tft.height();
        int tftwidth = tft.width();
        int frogwidth = ROAD_LENGTH - 10;

        if (roads[scroll + i] == 0)
        {
            // grass
            tft.fillRect(0, tftheight - (i * ROAD_LENGTH) - ROAD_LENGTH, tftwidth, ROAD_LENGTH, TFT_GREEN);
        }
        else
        {
            // road
            tft.fillRect(0, tftheight - (i * ROAD_LENGTH) - ROAD_LENGTH, tftwidth, ROAD_LENGTH, TFT_DARKGREY);

            // car
            tft.fillRect(cars[scroll + i].x,
                         tftheight - (i * ROAD_LENGTH) - ROAD_LENGTH + ((ROAD_LENGTH - frogwidth) / 2),
                         cars[scroll + i].width, frogwidth, cars[scroll + i].color);
        }

        if (score == scroll + i)
        {
            // frog
            tft.fillRect((tftwidth / 2) - (frogwidth / 2),
                         tftheight - (i * ROAD_LENGTH) - ROAD_LENGTH + ((ROAD_LENGTH - frogwidth) / 2),
                         frogwidth, frogwidth, TFT_DARKGREEN);
        }

        // draw score on road
        // tft.drawString(String(scroll + i), 0, tftheight - (i * ROAD_LENGTH) - ROAD_LENGTH);
    }
}

void generateRoads(int len)
{
    int carcolors[6] = {TFT_RED, TFT_PINK, TFT_SILVER, TFT_SKYBLUE, TFT_WHITE, TFT_BLUE};

    for (int i = 0; i < len; i++)
    {
        // Make sure road after screen scroll is grass
        if ((roadlength + i) % ROADS_ON_SCREEN == 0)
        {
            roads[roadlength + i] = 0;
        }
        else
        {
            roads[roadlength + i] = random(2);
            if (roads[roadlength + i] == 1)
            {
                cars[roadlength + i].color = carcolors[random(7)];
                cars[roadlength + i].width = 30 + random(50);
                cars[roadlength + i].speed = 5 + random(score / 2);
                cars[roadlength + i].moveright = ((random(2) % 2) == 0);
                cars[roadlength + i].x = random(tft.width());
                // cars[roadlength + i].x = cars[roadlength + i].moveright ? -100 : tft.width() + 100;
            }
        }
    }

    roadlength = roadlength + len;
    Serial.printf("generated %d roads, %d total\n", len, roadlength);
}

void moveCars()
{
    int beginningdraw = scroll - ROADS_ON_SCREEN < 0 ? 0 : scroll - ROADS_ON_SCREEN;
    for (int i = beginningdraw; i < scroll + ROADS_ON_SCREEN; i++)
    {
        if (roads[i] == 1)
        {
            if (cars[i].moveright)
            {
                cars[i].x += cars[i].speed;
            }
            else
            {
                cars[i].x -= cars[i].speed;
            }

            if (cars[i].x - cars[i].width > tft.width() && cars[i].moveright)
            {
                cars[i].x = -cars[i].width - random(100);
            }
            if (cars[i].x + cars[i].width < 0 && !cars[i].moveright)
            {
                cars[i].x = tft.width() + cars[i].width + random(100);
            }
        }
    }
}

bool checkForCollision()
{
    if (roads[score] == 1)
    {
        Car carInRoad = cars[score];
        int frogsize = ROAD_LENGTH - 10;
        int frogx = (tft.width() / 2) - (frogsize / 2);

        if (carInRoad.x + carInRoad.width > frogx && frogx + frogsize > carInRoad.x)
        {
            return true;
        }
    }

    return false;
}

void resetFrame()
{
    frame = 0;
    lastupdateframe = 0;
}

void btn1click()
{
    score++;
    if (score % ROADS_ON_SCREEN == 0 && score != 0)
    {
        scroll += ROADS_ON_SCREEN;
        generateRoads(ROADS_ON_SCREEN);
    }
}

void btn2click()
{
    if (!(score % ROADS_ON_SCREEN == 0))
    {
        score--;
    }
}

void died()
{
    alive = false;
    tft.setTextColor(TFT_RED);
    tft.setTextFont(1);
    tft.setTextSize(2);
    tft.drawCentreString("YOU DIED", tft.width() / 2, tft.height() / 2, 1);
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Ribbit Ribbit!");

    tft.init();
    tft.setRotation(4);
    tft.fillScreen(TFT_GREEN);

    roads[0] = 0;
    roadlength = 1;
    Serial.printf("road and car memory usage %d\n", sizeof(cars) + sizeof(roads));
    generateRoads(ROADS_ON_SCREEN);

    btn1.setClickHandler([](Button2 &b) { btn1click(); });
    btn2.setClickHandler([](Button2 &b) { btn2click(); });
}

void loop()
{
    if (alive)
    {
        drawRoad();
        drawScore();
    }

    if (checkForCollision())
        died();

    if (frame - lastupdateframe >= 10)
    {
        moveCars();
        lastupdateframe = frame;
    }

    if (frame == INT_MAX)
        resetFrame();

    if (alive)
    {
        btn1.loop();
        btn2.loop();
        frame++;
    }
}