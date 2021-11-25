#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits>
#include <limits.h>
#include <math.h>
using namespace std;

typedef struct RECT{
    float x1;
    float y1;
    float x2;
    float y2;
}RECT;

typedef struct POLYGON{
    long long int name;
    char mask;
    char layer[25];
    vector<RECT> rect;
    float x1;
    float y1;
    float x2;
    float y2;
    float width;
    float height;
    float center_x;
    float center_y;
    vector<long long int> conflicts;
}POLYGON;

typedef struct ROW{
    long long int name;
    float lowerleft_x;
    float lowerleft_y;
    vector<POLYGON> polygon;
}ROW;

void calculation();
void output();
void coloring();

ROW row;
POLYGON temp_polygon;
RECT temp_rect;
char temp[25], cur_row[25], cur_polygon[25];

float coloring_disatnce = 0;

FILE *rfp;
FILE *wfp;

char input_path[PATH_MAX];
char output_path[PATH_MAX];

main(int argc, char **argv)
{
    if (argc != 4) {
       printf("Usage: Hybrid_Lithography <Coloring distance> <Original testcase file path> <Your output file path>\n");
       return 1;
    }
    coloring_disatnce = atof(argv[1]);
    //cout << coloring_disatnce << endl;
    realpath(argv[2], input_path);
    //cout << input_path << endl;
    realpath(argv[3], output_path);
    //cout << output_path << endl;
    rfp = fopen(input_path,"r");
    wfp = fopen(output_path,"w");
    temp_polygon.mask = 'E';
    row.name = -1;
    temp_polygon.name = -1;
    while(fscanf(rfp, "%s",&temp)!=EOF)
    {
        //printf("%s\n",temp);
        if(strcmp(temp,"ROW") == 0)
        {
            //printf("HI\n");
            fscanf(rfp, "%s",&cur_row);
            row.name = row.name + 1;
            //temp_polygon.name = -1;
            fscanf(rfp, "%f %f",&row.lowerleft_x, &row.lowerleft_y);
            while(1)
            {
                fscanf(rfp, "%s",&temp);
                if(strcmp(temp,cur_row) == 0)
                {
                    calculation();
                    coloring();
                    output();
                    row.polygon.clear();
                    break;
                }
                else if(strcmp(temp,"POLYGON") == 0)
                {
                    fscanf(rfp, "%s",&cur_polygon);
                    temp_polygon.name = temp_polygon.name + 1;
                    fscanf(rfp, "%s",&temp);
                    fscanf(rfp, "%s",&temp);
                    fscanf(rfp, "%s",&temp);
                    fscanf(rfp, "%s",&temp_polygon.layer);
                    temp_polygon.x1 = numeric_limits<float>::max();
                    temp_polygon.y1 = numeric_limits<float>::max();
                    temp_polygon.x2 = numeric_limits<float>::min();
                    temp_polygon.y2 = numeric_limits<float>::min();
                    while(1)
                    {
                        fscanf(rfp, "%s",&temp);
                        if(strcmp(temp,cur_polygon) == 0)
                        {
                            temp_polygon.width = temp_polygon.x2 - temp_polygon.x1;
                            temp_polygon.height = temp_polygon.y2 - temp_polygon.y1;
                            temp_polygon.center_x = (temp_polygon.x2 + temp_polygon.x1)/2;
                            temp_polygon.center_y = (temp_polygon.y2 + temp_polygon.y1)/2;
                            row.polygon.push_back(temp_polygon);
                            temp_polygon.rect.clear();
                            break;
                        }
                        else if(strcmp(temp,"RECT") == 0)
                        {
                            fscanf(rfp, "%f %f %f %f",&temp_rect.x1,&temp_rect.y1,&temp_rect.x2,&temp_rect.y2);
                            if(temp_polygon.x1 > temp_rect.x1)
                            {
                                temp_polygon.x1 = temp_rect.x1;
                            }
                            if(temp_polygon.y1 > temp_rect.y1)
                            {
                                temp_polygon.y1 = temp_rect.y1;
                            }
                            if(temp_polygon.x2 < temp_rect.x2)
                            {
                                temp_polygon.x2 = temp_rect.x2;
                            }
                            if(temp_polygon.y2 < temp_rect.y2)
                            {
                                temp_polygon.y2 = temp_rect.y2;
                            }
                            temp_polygon.rect.push_back(temp_rect);
                        }
                    }
                }
            }
        }
    }
    fclose(rfp);
    fclose(wfp);
    return 0;
}

void output()
{
    fprintf(wfp, "ROW R%lld %.7g %.7g \n",row.name,row.lowerleft_x,row.lowerleft_y);
    long long int sizeofpolygon = row.polygon.size();
    for(long long int i=0;i<sizeofpolygon;i++)
    {
        fprintf(wfp, "\tPOLYGON P%lld\n",row.polygon[i].name);
        fprintf(wfp, "\t\tMASK %c\n",row.polygon[i].mask);
        fprintf(wfp, "\t\tLAYER %s\n",row.polygon[i].layer);
        long long int sizeofrect = row.polygon[i].rect.size();
        for(long long int j=0;j<sizeofrect;j++)
        {
            fprintf(wfp, "\t\t\tRECT %.7g %.7g %.7g %.7g\n",row.polygon[i].rect[j].x1,row.polygon[i].rect[j].y1,row.polygon[i].rect[j].x2,row.polygon[i].rect[j].y2);
        }
        fprintf(wfp, "\t\tEND\n");
        fprintf(wfp, "\tEND P%lld\n",row.polygon[i].name);
    }
    fprintf(wfp, "END R%lld\n",row.name);
}

void calculation()
{
    long long int sizeofpolygon = row.polygon.size();
    long long int i,j;
    float min_dist, Dx, Dy;
    for(i=0;i<sizeofpolygon;i++)
    {
        for(j=i+1;j<sizeofpolygon;j++)
        {
            Dx = abs(row.polygon[j].center_x - row.polygon[i].center_x);
            Dy = abs(row.polygon[j].center_y - row.polygon[i].center_y);

            if((Dx < ((row.polygon[i].width + row.polygon[j].width)/ 2)) && (Dy >= ((row.polygon[i].height + row.polygon[j].height) / 2)))
            {
                min_dist = Dy - ((row.polygon[i].height + row.polygon[j].height) / 2);
            }
            else if((Dx >= ((row.polygon[i].width + row.polygon[j].width)/ 2)) && (Dy < ((row.polygon[i].height + row.polygon[j].height) / 2)))
            {
                min_dist = Dx - ((row.polygon[i].width + row.polygon[j].width)/ 2);
            }
            else if((Dx >= ((row.polygon[i].width + row.polygon[j].width)/ 2)) && (Dy >= ((row.polygon[i].height + row.polygon[j].height) / 2)))
            {
                float delta_x = Dx - ((row.polygon[i].width + row.polygon[j].width)/ 2);
                float delta_y = Dy - ((row.polygon[i].height + row.polygon[j].height)/ 2);
                min_dist = sqrt(delta_x * delta_x  + delta_y * delta_y);
            }
            else
            {
                min_dist = -1;
            }

            if(min_dist < coloring_disatnce)
            {
                row.polygon[i].conflicts.push_back(j);
                row.polygon[j].conflicts.push_back(i);
            }
        }
    }
}

void coloring()
{
    long long int sizeofpolygon = row.polygon.size();
    for(int i=0;i<sizeofpolygon;i++)
    {
        bool color_A = true;
        bool color_B = true;
        bool color_C = true;
        long long int sizeofconflicts = row.polygon[i].conflicts.size();
        for(int j=0;j<sizeofconflicts;j++)
        {
            long long int c = row.polygon[i].conflicts[j];
            char M = row.polygon[c].mask;
            if(M == 'A')
            {
                color_A = false;
            }
            else if(M == 'B')
            {
                color_B = false;
            }
            else if(M == 'C')
            {
                color_C = false;
            }
        }
        if(color_A)
        {
            row.polygon[i].mask = 'A';
        }
        else if(color_B)
        {
            row.polygon[i].mask = 'B';
        }
        else if(color_C)
        {
            row.polygon[i].mask = 'C';
        }
        else
        {
            row.polygon[i].mask = 'E';
        }
    }
}
