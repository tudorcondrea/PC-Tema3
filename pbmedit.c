#include "pbmedit.h"

int minimum(int a, int b)
{
    if (a < b)
        return a;
    else
        return b;
}

triplet ** load(char filename[], corner *c_limits, int * intensity, int * magic_word)
{
    triplet ** img = NULL;
    FILE * img_file = fopen(filename, "r");
    int size_set = 0, intensity_set = 0, line_i = 0, h_offset = 0, n, m;
    char iter, temp_char, line[1000];
    if (img_file != NULL)
    {
        do{
            if (*magic_word >= 4 && *magic_word <= 6 && size_set == 1 && intensity_set == 1)
            {
                h_offset = ftell(img_file);
                fclose(img_file);
                img_file = fopen(filename, "rb");
                fseek(img_file, h_offset, SEEK_SET);
            }
            iter = fgetc(img_file);
            if (iter == 'P')
            {
                fscanf(img_file, "%c", &temp_char);
                *magic_word = temp_char - '0';
                if (*magic_word == 1 || *magic_word == 4)
                    intensity_set = 1;
            }
            else if (isdigit(iter))
            {
                fseek(img_file, -1, SEEK_CUR);
                if (size_set == 0)
                {
                    size_set = 1;
                    fscanf(img_file, "%d%d", &m, &n);
                    img = (triplet **)calloc(n, sizeof(triplet *));
                    for (int i = 0; i < n; i++)
                        img[i] = (triplet *)calloc(m, sizeof(triplet));
                    *c_limits = resize(0, 0, n - 1, m - 1);
                }
                else
                    if (intensity_set == 0)
                    {
                        intensity_set = 1;
                        fscanf(img_file, "%d", intensity);
                    }
                
            }
            else if (iter == '#')
                fgets(line, 1000, img_file);
        }while(size_set == 0 || intensity_set == 0);
        fgetc(img_file);
        if (*magic_word >= 1 && *magic_word <= 3)
        {
            do{
                iter = fgetc(img_file);
                if (iter == '#')
                    fgets(line, 1000, img_file);
                else if (isdigit(iter))
                {
                    fseek(img_file, -1, SEEK_CUR);
                    for (int i = 0; i < m; i++)
                        if (*magic_word == 1 || *magic_word == 2)
                            fscanf(img_file, "%d", &img[line_i][i].r);
                        else
                            fscanf(img_file, "%d%d%d", &img[line_i][i].r, &img[line_i][i].g, &img[line_i][i].b);
                    line_i++;
                }
            }while(iter != EOF);
        }
        else
        {
            for (int i = 0; i < n; i++)
                for (int j = 0; j < m; j++)
                    if (*magic_word == 4 || *magic_word == 5)
                        fread(&img[i][j].r, 1, 1, img_file);
                    else
                    {
                        fread(&img[i][j].r, 1, 1, img_file);
                        fread(&img[i][j].g, 1, 1, img_file);
                        fread(&img[i][j].b, 1, 1, img_file);
                    }
        }
        fclose(img_file);
    }
    return img;
}

corner resize(int x1, int y1, int x2, int y2)
{
    corner temp;
    temp.min.x = x1;
    temp.min.y = y1;
    temp.max.x = x2;
    temp.max.y = y2;
    return temp;
}

triplet ** transpose(triplet ** img, corner *cor)
{
    triplet ** trans_img = (triplet **)calloc(((*cor).max.y - (*cor).min.y + 1), sizeof(triplet *));
    for (int i = 0; i <= (*cor).max.y - (*cor).min.y; i++)
        trans_img[i] = (triplet *)calloc(((*cor).max.x - (*cor).min.x + 1), sizeof(triplet));
    for (int i = 0; i <= (*cor).max.x - (*cor).min.x; i++)
        for (int j = 0; j <= (*cor).max.y - (*cor).min.y; j++)
        {
            //printf("%d %d <=> %d %d\n", j, i, i + (*cor).min.x, j + (*cor).min.y);
            trans_img[j][i] = img[i + (*cor).min.x][j + (*cor).min.y];
        }
    int aux = (*cor).min.x;
    (*cor).min.x = (*cor).min.y;
    (*cor).min.y = aux;
    aux = (*cor).max.x;
    (*cor).max.x = (*cor).max.y;
    (*cor).max.y = aux;
    return trans_img;
}

void swapRows(triplet ** img, corner cor)
{
    for (int i = cor.min.x; i <= (cor.max.x + cor.min.x) / 2; i++)
        for (int j = cor.min.y; j <= cor.max.y; j++)
        {
            //printf("%d %d %d %d\n", i, j, cor.max.x - i + cor.min.x, j);
            triplet aux = img[i][j];
            img[i][j] = img[cor.max.x - i + cor.min.x][j];
            img[cor.max.x - i + cor.min.x][j] = aux;
        }
    //printf("\n");
}

int validate_corners(corner c, corner c_limits)
{
    if (c.min.x < 0 || c.min.x > c_limits.max.x || c.min.y < 0 || c.min.y > c_limits.max.y || c.max.x < 0 || c.max.x > c_limits.max.x || c.max.y < 0 || c.max.y > c_limits.max.y)
        {
            //printf("%d %d %d %d\n%d %d %d %d\n", c.min.x, c.min.y, c.max.x, c.max.y, c_limits.min.x, c_limits.min.y, c_limits.max.x, c_limits.max.y);
            return 0;
        }
    else
        return 1;
}

triplet ** crop(triplet ** img, corner * c_crop, corner c_restrict)
{
    triplet ** img_crop;
    (*c_crop).min.x = (*c_crop).min.y = 0;
    (*c_crop).max.x = c_restrict.max.x - c_restrict.min.x;
    (*c_crop).max.y = c_restrict.max.y - c_restrict.min.y;
    img_crop = (triplet **)calloc(((*c_crop).max.x + 1), sizeof(triplet *));
    for (int i = 0; i <= (*c_crop).max.x; i++)
        img_crop[i] = (triplet *)calloc(((*c_crop).max.y + 1), sizeof(triplet));
    for (int i = 0; i <= (*c_crop).max.x; i++)
        for (int j = 0; j <= (*c_crop).max.y; j++)
            img_crop[i][j] = img[i + c_restrict.min.x][j + c_restrict.min.y];
    return img_crop;
}

void grayscale(triplet ** img, corner c_restrict)
{
    for (int i = c_restrict.min.x; i <= c_restrict.max.x; i++)
        for (int j = c_restrict.min.y; j <= c_restrict.max.y; j++)
        {
            int gray = (img[i][j].r + img[i][j].g + img[i][j].b) / 3;
            img[i][j].r = img[i][j].g = img[i][j].b = gray;
        }
}

void sepia(triplet ** img, corner c_restrict)
{
    for (int i = c_restrict.min.x; i <= c_restrict.max.x; i++)
        for (int j = c_restrict.min.y; j <= c_restrict.max.y; j++)
        {
            int new_color = minimum(0.393 * img[i][j].r + 0.769 * img[i][j].g + 0.189 * img[i][j].b, 255);
            img[i][j].r = new_color;
            new_color = minimum(0.349 * img[i][j].r + 0.686 * img[i][j].g + 0.168 * img[i][j].b, 255);
            img[i][j].g = new_color;
            new_color = minimum(0.272 * img[i][j].r + 0.534 * img[i][j].g + 0.131 * img[i][j].b, 255);
            img[i][j].b = new_color;
        }
}

void save(char filename[], int magic_word, corner c_limits, int intensity, triplet ** img, int ascii)
{
    FILE * out = fopen(filename, "wb");
    int new_magic_word;
    if (ascii == 1)
    {
        if (magic_word == 1 || magic_word == 2 || magic_word == 3)
            new_magic_word = magic_word;
        else
            new_magic_word = magic_word - 3;
    }
    else
    {
        if (magic_word == 4 || magic_word == 5 || magic_word == 6)
            new_magic_word = magic_word;
        else
            new_magic_word = magic_word + 3;
    }
    fprintf(out, "P%d\n%d %d\n%d\n", new_magic_word, c_limits.max.y + 1, c_limits.max.x + 1, intensity);
    if (ascii == 1)
        for (int i = 0; i <= c_limits.max.x; i++)
        {
            for (int j = 0; j <= c_limits.max.y; j++)
                if (magic_word == 1 || magic_word == 2 || magic_word == 4 || magic_word == 5)
                    fprintf(out, "%d ", img[i][j].r);
                else if (magic_word == 3 || magic_word == 6)
                {
                    fprintf(out, "%d %d %d ", img[i][j].r, img[i][j].g, img[i][j].b);
                }
            fprintf(out, "\n");
        }
    else
        for (int i = 0; i <= c_limits.max.x; i++)
        {
            for (int j = 0; j <= c_limits.max.y; j++)
                if (magic_word == 1 || magic_word == 2 || magic_word == 4 || magic_word == 5)
                    fwrite(&img[i][j].r, 1, 1, out);
                else if (magic_word == 3 || magic_word == 6)
                {
                    fwrite(&img[i][j].r, 1, 1, out);
                    fwrite(&img[i][j].g, 1, 1, out);
                    fwrite(&img[i][j].b, 1, 1, out);
                }
        }
    fclose(out);
}