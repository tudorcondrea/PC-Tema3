#include "pbmedit.h"
//functie pentru citit comanda care intoarce 1 daca e valida, 0 daca nu e valida
int get_cmd(char * command, char params[30][60])
{
    for (int i = 0; i < 30; i++)
        params[i][0] = 0;
    char line[300], *tok;
    fgets(line, 300, stdin);
    if (strcmp(line, "\n") == 0)
        return 0;
    if (line[strlen(line) - 1] == '\n')
        line[strlen(line) - 1] = '\0';
    tok = strtok(line, " ");
    memcpy(command, tok, strlen(tok) + 1);
    int index = 0;
    while(tok != NULL)
    {
        tok = strtok(NULL, " ");
        if (tok != NULL)
        {
            memcpy(params[index], tok, strlen(tok) + 1);
            index++;
        }
    }
    index--;
    //verificare pentru diferite lucruri care ar face o comanda nevalida
    int ok = 0;
    if (strcmp(command, "LOAD") == 0 || strcmp(command, "SELECT") == 0 || strcmp(command, "SELECT ALL") == 0 || strcmp(command, "ROTATE") == 0 || strcmp(command, "CROP") == 0 || strcmp(command, "GRAYSCALE") == 0 || strcmp(command, "SEPIA") == 0 || strcmp(command, "SAVE") == 0 || strcmp(command, "EXIT") == 0)
        ok = 1;
    if (strcmp(command, "SAVE") == 0 && index == 1 && strcmp(params[1], "ascii") != 0)
        ok = 0;
    if (strcmp(command, "SELECT") == 0 && strcmp(params[0], "ALL") != 0)
    {
        if (index < 3)
            ok = 0;
        else
            for (int i = 0; i < 4; i++)
                for (long unsigned int j = 0; j < strlen(params[i]); j++)
                    if (isdigit(params[i][j]) == 0 && params[i][j] != '-')
                        ok = 0;
    }
    if (strcmp(command, "ROTATE") == 0)
        for (long unsigned int i = 0; i < strlen(params[0]); i++)
            if (isdigit(params[0][i] == 0) && params[0][i] != '-')
                ok = 0;
    return ok;
}
//returneaza 1 daca limitile sunt egale, 0 daca nu sunt
int compare_corners(corner c1, corner c2)
{
    if (c1.min.x != c2.min.x || c1.min.y != c2.min.y || c1.max.x != c2.max.x || c1.max.y != c2.max.y)
        return 0;
    else
        return 1;
}

int main(void)
{
    char command[20], params[30][60];
    int magic_word = 0, intensity = 0, loaded = 0, cmd_signal = 0;
    triplet ** img = NULL;
    corner c_limits, c_restrict, c_aux;
    while (1)
    {
        cmd_signal = get_cmd(command, params);
        if (cmd_signal == 0)
            printf("Invalid command\n");
        else
        {
            //fiecare comanda are o logica separata efectuata cu ajutorul unor functii
            if (strcmp(command, "LOAD") == 0)
            {
                if (loaded == 1)
                {
                    for (int i = 0; i <= c_limits.max.x; i++)
                        free(img[i]);
                    free(img);
                }
                img = load(params[0], &c_limits, &intensity, &magic_word);
                if (img != NULL)
                {
                    loaded = 1;
                    c_restrict = c_limits;
                    printf("Loaded %s\n", params[0]);
                }
                else
                {
                    loaded = 0;
                    printf("Failed to load %s\n", params[0]);
                }
            }
            else if (strcmp(command, "SELECT") == 0)
            {
                if (img == NULL)
                    printf("No image loaded\n");
                else
                {
                    //daca vrea o selectie specifica
                    if (strcmp(params[0], "ALL") != 0)
                    {
                        //corectam inputurile inversate
                        //functiile de editare functioneaza cu o abordare de puncte in spatiu, fata de intervale
                        c_aux = resize(atoi(params[1]), atoi(params[0]), atoi(params[3]), atoi(params[2]));
                        if (c_aux.min.x > c_aux.max.x)
                        {
                            c_aux = resize(c_aux.max.x, c_aux.min.y, c_aux.min.x, c_aux.max.y);
                            char temp_str[50];
                            strcpy(temp_str, params[1]);
                            strcpy(params[1], params[3]);
                            strcpy(params[3], temp_str);
                        }
                        if(c_aux.min.y > c_aux.max.y)
                        {
                            c_aux = resize(c_aux.min.x, c_aux.max.y, c_aux.max.x, c_aux.min.y);
                            char temp_str[50];
                            strcpy(temp_str, params[0]);
                            strcpy(params[0], params[2]);
                            strcpy(params[2], temp_str);
                        }
                        //nu accepta linii sau puncte de grosime 0
                        if (c_aux.min.x == c_aux.max.x || c_aux.min.y == c_aux.max.y)
                            printf("Invalid set of coordinates\n");
                        else
                        {
                            c_aux = resize(c_aux.min.x, c_aux.min.y, c_aux.max.x - 1, c_aux.max.y - 1);
                            if (validate_corners(c_aux, c_limits) == 0)
                                printf("Invalid set of coordinates\n");
                            else
                            {
                                c_restrict = c_aux;
                                printf("Selected %d %d %d %d\n", atoi(params[0]), atoi(params[1]), atoi(params[2]), atoi(params[3]));
                            }
                        }
                    }
                    else
                    {
                        c_restrict = c_limits;
                        printf("Selected ALL\n");
                    }
                }
            }
            else if (strcmp(command, "ROTATE") == 0)
            {
                //pentru rotatia la dreapta folosesc transpusa randurile oglindite anterior
                //pentru rotatia la stanga folosesc transpusa la care ii oglindesc randurile
                int angle = atoi(params[0]);
                if (img == NULL)
                    printf("No image loaded\n");
                else
                {
                    if (angle % 90 != 0)
                        printf("Unsupported rotation angle\n");
                    else
                    {
                        if (compare_corners(c_restrict, c_limits) == 1)
                        {
                            if (angle > 0)
                                for (int i = 0; i < angle % 360 / 90; i++)
                                {
                                    corner c_trans = c_limits;
                                    swapRows(img, c_limits);
                                    triplet ** img_trans = transpose(img, &c_trans);
                                    for (int i = 0; i <= c_limits.max.x; i++)
                                        free(img[i]);
                                    free(img);
                                    c_restrict = c_limits = c_trans;
                                    img = img_trans;
                                }
                            else
                                for (int i = 0; i < -1* (angle %360) / 90; i++)
                                {
                                    corner c_trans = c_limits;
                                    triplet ** img_trans = transpose(img, &c_trans);
                                    swapRows(img_trans, c_trans);
                                    for (int i = 0; i <= c_limits.max.x; i++)
                                        free(img[i]);
                                    free(img);
                                    c_limits = c_restrict = c_trans;
                                    img = img_trans;
                                }
                            printf("Rotated %d\n", angle);
                        }
                        else
                        {
                            if (c_restrict.max.x - c_restrict.min.x == c_restrict.max.y - c_restrict.min.y)
                            {
                                if (angle > 0)
                                    for (int i = 0; i < angle % 360 / 90; i++)
                                    {
                                        swapRows(img, c_restrict);
                                        triplet ** img_trans = transpose(img, &c_restrict);
                                        c_restrict = resize(c_restrict.min.y, c_restrict.min.x, c_restrict.max.y, c_restrict.max.x);
                                        for (int i = c_restrict.min.x; i <= c_restrict.max.x; i++)
                                            for (int j = c_restrict.min.y; j <= c_restrict.max.y; j++)
                                                    img[i][j] = img_trans[i - c_restrict.min.x][j - c_restrict.min.y];
                                        for (int i = 0; i <= c_restrict.max.x - c_restrict.min.x; i++)
                                            free(img_trans[i]);
                                        free(img_trans);
                                    }
                                else
                                    for (int i = 0; i < -1*(angle % 360) / 90; i++)
                                    {
                                        triplet ** img_trans = transpose(img, &c_restrict);
                                        c_restrict = resize(c_restrict.min.y, c_restrict.min.x, c_restrict.max.y, c_restrict.max.x);
                                        c_aux = resize(0, 0, c_restrict.max.x - c_restrict.min.x, c_restrict.max.y - c_restrict.min.y);
                                        swapRows(img_trans, c_aux);
                                        for (int i = c_restrict.min.x; i <= c_restrict.max.x; i++)
                                            for (int j = c_restrict.min.y; j <= c_restrict.max.y; j++)
                                            {
                                                img[i][j] = img_trans[i - c_restrict.min.x][j - c_restrict.min.y];
                                            }
                                        for (int i = 0; i <= c_aux.max.x; i++)
                                            free(img_trans[i]);
                                        free(img_trans);
                                    }
                                printf("Rotated %d\n", angle);
                            }
                            else
                                printf("The selection must be square\n");
                        }
                    }
                }
            }
            else if (strcmp(command, "CROP") == 0)
            {
                if (img == NULL)
                    printf("No image loaded\n");
                else
                {
                    corner c_crop;
                    triplet ** img_crop = crop(img, &c_crop, c_restrict);
                    for (int i = 0; i <= c_limits.max.x; i++)
                        free(img[i]);
                    free(img);
                    img = img_crop;
                    c_limits = c_crop;
                    c_restrict = c_crop;
                    printf("Image cropped\n");
                }
            }
            else if (strcmp(command, "GRAYSCALE") == 0)
            {
                if (img == NULL)
                    printf("No image loaded\n");
                else
                {
                    if (magic_word != 3 && magic_word != 6)
                        printf("Grayscale filter not available\n");
                    else
                    {
                        grayscale(img, c_restrict);
                        printf("Grayscale filter applied\n");
                    }
                }
            }
            else if (strcmp(command, "SEPIA") == 0)
            {
                if (img == NULL)
                    printf("No image loaded\n");
                else
                {
                    if (magic_word != 3 && magic_word != 6)
                        printf("Sepia filter not available\n");
                    else
                    {
                        sepia(img, c_restrict);
                        printf("Sepia filter applied\n");
                    }
                }
            }
            else if (strcmp(command, "SAVE") == 0)
            {
                if (img == NULL)
                    printf("No image loaded\n");
                else
                {
                    if (strcmp(params[1], "ascii") == 0)
                        save(params[0], magic_word, c_limits, intensity, img, 1);
                    else
                        save(params[0], magic_word, c_limits, intensity, img, 0);
                    printf("Saved %s\n", params[0]);
                }
            }
            else if (strcmp(command, "EXIT") == 0)
            {
                if (img != NULL)
                {
                    for (int i = 0; i <= c_limits.max.x; i++)
                        free(img[i]);
                    free(img);
                }
                else
                    printf("No image loaded\n");
                return 0;
            }
        }
    }
    return 0;
}