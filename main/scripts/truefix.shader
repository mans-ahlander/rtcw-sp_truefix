truefix_trigger
{
    cull disable
    sort blend

    {
        map $whiteimage
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen const ( 0.0 1.0 0.0 )
        alphaGen const 0.20
    }
}