float4 main() : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}

/*
//x and y offset based off
private static double MANDELBROT_RE_MIN = -2;
private static double MANDELBROT_RE_MAX = 1;
private static double MANDELBROT_IM_MIN = -1.2;
private static double MANDELBROT_IM_MAX = 1.2;


public void run()
{
    for (double xR = start, c = initPrecision; xR < stop; c = c + stdPrecision, xR++)
    {
        for (double yR = 0, ci = imMin; yR < CANVAS_HEIGHT; ci = ci + stdPrecision, yR++)
        {
            double convergenceValue = convergenceSteps;
            double z = 0;
            double zi = 0;
            for (int i = 0; i < convergenceSteps; i++)
            {
                double ziT = 2 * (z * zi);
                double zT = z * z - (zi * zi);
                z = zT + c;
                zi = ziT + ci;
                if (z * z + zi * zi >= 4.0)
                {
                    convergenceValue = i;
                    break;
                }
            }

            double t1 = convergenceValue / convergenceSteps;
            double c1 = Math.min(255 * 2 * t1, 255);
            double c2 = Math.max(255 * (2 * t1 - 1), 0);

            if (convergenceValue != convergenceSteps)
                ctx.setFill(Color.color(c2 / 255.0, c1 / 255.0, c2 / 255.0));
            else
                ctx.setFill(Color.PURPLE);

            //after the colour has been set then draw the colour here
            //  fill in a single pixel
            ctx.fillRect(xR, yR, 1, 1);
        }
    }
}

private void paintSet(GraphicsContext ctx, double reMin, double reMax, double imMin, double imMax)
{
    double precision = Math.max((reMax - reMin) / CANVAS_WIDTH, (imMax - imMin) / CANVAS_HEIGHT);
    int convergenceSteps = 50;
    int number_of_threads = 8;

    //the size of each portion based off the thread count
    int size = CANVAS_WIDTH / number_of_threads;
    //how many pixels are remaining that will be picked up by the first thread
    double remainder = (double)CANVAS_WIDTH % number_of_threads;

    for (double i = 0, start = 0, stop = size; i < number_of_threads; start += size, stop += size, i++)
    {
        if (i == number_of_threads - 1)
            stop += remainder;

        double initPrecision = reMin + (precision * start);
        handoff.ExecutionThread(convergenceSteps, imMin, initPrecision, precision, CANVAS_HEIGHT, start, stop);
    }
}
*/