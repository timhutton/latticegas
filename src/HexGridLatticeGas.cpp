#include "HexGridLatticeGas.h"

HexGridLatticeGas::HexGridLatticeGas()
{
    {
        const int tmp_NBORS[2][N_DIRS][2] = {
            {{1,0},{0,1},{-1,1},{-1,0},{-1,-1},{0,-1}},
            {{1,0},{1,1},{0,1}, {-1,0},{0,-1}, {1,-1}}
        };
        this->NBORS.assign(2,vector<vector<int> >(N_DIRS,vector<int>(2)));
        for(int rp=0;rp<2;rp++)
            for(int d=0;d<N_DIRS;d++)
                for(int xy=0;xy<2;xy++)
                    this->NBORS[rp][d][xy] = tmp_NBORS[rp][d][xy];
    }

    // directions run clockwise from East
    for(int i=0;i<N_DIRS;i++)
    {
        DIR[i][0] = cos(i*2*3.1415926535/N_DIRS);
        DIR[i][1] = sin(i*2*3.1415926535/N_DIRS);
    }
}

void HexGridLatticeGas::RedrawImagesIfNeeded()
{
    if(!this->need_redraw_images) return;

    float side = this->zoom_factor_num / (float)this->zoom_factor_denom;

    if(this->show_gas)
    {
        if(this->zoom_factor_denom==1) // ie. zoomed in enough to see cells
        {
            for(int x=0;x<X;x++)
            {
                for(int y=0;y<Y;y++)
                {
                    wxColour c = GetColour(x,y);
                    // draw a filled rect (quickest this way)
                    int x_offset = (y%2)?side/4:-side/4;
                    int from_x = x * side + x_offset;
                    int from_y = y * side;
                    int to_x = from_x + side;
                    int to_y = from_y + side;
                    for(int i=max(0,from_x);i<min(this->gas_image.GetWidth()-1,to_x);i++)
                    {
                        for(int j=max(0,from_y);j<min(this->gas_image.GetHeight()-1,to_y);j++)
                        {
						    // draw grid if zoomed in enough
						    if(this->show_grid && this->zoom_factor_num>=4 && (i==from_x || j==from_y))
							    gas_image.SetRGB(i,j,grid_lines_colour.Red(),grid_lines_colour.Green(),grid_lines_colour.Blue());
						    else
							    gas_image.SetRGB(i,j,c.Red(),c.Green(),c.Blue());
                        }
                    }
                }
            }
        }
        else // zoomed out beyond 1 pixel: show the density of the gas
        {
            // for each pixel:
            for(int px=0;px<this->drawing_bitmap.GetWidth();px++)
            {
                for(int py=0;py<this->drawing_bitmap.GetHeight();py++)
                {
                    int x = px * this->zoom_factor_denom;
                    int y = py * this->zoom_factor_denom;
                    // find the average colour for the cells in this region
                    int r=0,g=0,b=0,n=0;
                    for(int i=x;i<min(x+this->zoom_factor_denom,X-1);i++)
                    {
                        for(int j=y;j<min(y+this->zoom_factor_denom,Y-1);j++)
                        {
                            wxColour c = GetColour(i,j);
                            r+=c.Red();
                            g+=c.Green();
                            b+=c.Blue();
                            n++;
                        }
                    }
                    wxColour c = wxColour(r/n,g/n,b/n);
                    gas_image.SetRGB(px,py,c.Red(),c.Green(),c.Blue());
                }
            }
        }
        this->drawing_buffer.DrawBitmap(wxBitmap(gas_image),0,0);
    }
    else // show_gas==false
    {
        this->drawing_buffer.SetBackground(*wxWHITE_BRUSH);
        this->drawing_buffer.Clear();
    }

    if(show_flow)
    {
        // draw the flow image
        if(need_recompute_flow)
            ComputeFlow();

        // draw flow vectors
        if(!this->show_flow_colours)
            this->drawing_buffer.SetPen(*wxBLACK_PEN);
        for(int x=this->flow_sample_separation;x<X-this->flow_sample_separation;x+=this->flow_sample_separation)
        {
            for(int y=this->flow_sample_separation;y<Y-this->flow_sample_separation;y+=this->flow_sample_separation)
            {
                int sx=x/this->flow_sample_separation,sy=y/this->flow_sample_separation;
                RealPoint v(velocity[sx][sy]);
                if(this->velocity_representation == Velocity_SubtractGlobalMean)
                {
                    // we subtract the averaged velocity at this point, to better highlight the dynamic changes
                    v.x -= global_mean_velocity.x;
                    v.y -= global_mean_velocity.y;
                }
                else if(this->velocity_representation == Velocity_SubtractPointMean)
                {
                    // we subtract the averaged velocity at this point, to better highlight the dynamic changes
                    v.x -= averaged_velocity[sx][sy].x;
                    v.y -= averaged_velocity[sx][sy].y;
                }
                if(this->show_flow_colours)
                    this->drawing_buffer.SetPen(wxPen(GetVectorAngleColour(v.x,v.y)));
                int x_offset = (y%2)?side/4:-side/4;
                this->drawing_buffer.DrawLine(
                    (x+0.5) * side + x_offset,
                    (y+0.5) * side,
                    (x+0.5+v.x*this->line_length)*side + x_offset,
                    (y+0.5+v.y*this->line_length)*side);
            }
        }
    }

    this->need_redraw_images = false;
}

RealPoint HexGridLatticeGas::GetVelocity(state s) const
{
    RealPoint v;
    for(int dir=0;dir<N_DIRS;dir++)
    {
        if(s & (1<<dir))
            v += RealPoint(DIR[dir][0],DIR[dir][1]);
    }
    return v;
}
