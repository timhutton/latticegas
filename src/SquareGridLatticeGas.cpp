#include "SquareGridLatticeGas.h"

void SquareGridLatticeGas::RedrawImagesIfNeeded()
{
    if(!this->need_redraw_images) return;

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
                    for(int i=x * this->zoom_factor_num / this->zoom_factor_denom;i<(x+1) * this->zoom_factor_num / this->zoom_factor_denom;i++)
                    {
                        for(int j=y*this->zoom_factor_num / this->zoom_factor_denom;j<(y+1)*this->zoom_factor_num / this->zoom_factor_denom;j++)
                        {
						    // draw grid if zoomed in enough
						    if(this->show_grid && this->zoom_factor_num>=4 && 
							    (i==x * this->zoom_factor_num / this->zoom_factor_denom ||
							    j==y * this->zoom_factor_num / this->zoom_factor_denom ) )
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
                this->drawing_buffer.DrawLine((x+0.5) * this->zoom_factor_num / this->zoom_factor_denom,
                    (y+0.5) * this->zoom_factor_num / this->zoom_factor_denom,
                    (x+0.5+v.x*this->line_length)*this->zoom_factor_num / this->zoom_factor_denom,
                    (y+0.5+v.y*this->line_length)*this->zoom_factor_num / this->zoom_factor_denom);
            }
        }
    }

    /* we can save images to make an animation but this is currently dormant
    if(this->save_images)
    {
        SetStatusTextHelper sth(_("Saving image snapshot..."),this);
        this->drawing_bitmap.SaveFile(wxString::Format(_("%04d.jpg"),this->n_saved_images++),wxBITMAP_TYPE_JPEG);
    }*/

    this->need_redraw_images = false;
}

