#include <iostream>
#include <string>
#include <opencv2\opencv.hpp>
#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\videoio.hpp>

extern "C" {
#include "vc.h"
}

void vc_timer(void) {
    static bool running = false;
    static std::chrono::steady_clock::time_point previousTime = std::chrono::steady_clock::now();

    if (!running) {
        running = true;
    }
    else {
        std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
        std::chrono::steady_clock::duration elapsedTime = currentTime - previousTime;

        // Tempo em segundos.
        std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(elapsedTime);
        double nseconds = time_span.count();

        std::cout << "Tempo decorrido: " << nseconds << " segundos" << std::endl;
        std::cout << "Pressione qualquer tecla para continuar...\n";
        std::cin.get();
    }
}

int main(void) {
    // Vídeo
    char videofile[20] = "video1.mp4";
    cv::VideoCapture capture;
    struct
    {
        int width, height;
        int ntotalframes;
        int fps;
        int nframe;
    } video;
    std::string str;
    int key = 0;

    OVC* blob = nullptr;
    int nblob = 0;

    int raio = 0;
    int x_width = 0;
    int y_height = 0;
    int width_res = 0;
    int height_res = 60;
    int xmin = 0;
    int xmax = 0;

    int nMoedas = 0;

    /* Leitura de v deo de um ficheiro */
    /* NOTA IMPORTANTE:
    O ficheiro video.avi dever  estar localizado no mesmo direct rio que o ficheiro de c digo fonte.
    */
    capture.open(videofile);

    /* Em alternativa, abrir captura de v deo pela Webcam #0 */
    //capture.open(0, cv::CAP_DSHOW); // Pode-se utilizar apenas capture.open(0);

    /* Verifica se foi poss vel abrir o ficheiro de v deo */
    if (!capture.isOpened())
    {
        std::cerr << "Erro ao abrir o ficheiro de v deo!\n";
        return 1;
    }

    /* N mero total de frames no v deo */
    video.ntotalframes = (int)capture.get(cv::CAP_PROP_FRAME_COUNT);
    /* Frame rate do v deo */
    video.fps = (int)capture.get(cv::CAP_PROP_FPS);
    /* Resolu  o do v deo */
    video.width = (int)capture.get(cv::CAP_PROP_FRAME_WIDTH);
    video.height = (int)capture.get(cv::CAP_PROP_FRAME_HEIGHT);


    /* Cria uma janela para exibir o v deo */
    cv::namedWindow("VC - VIDEO", cv::WINDOW_AUTOSIZE);
    //cv::namedWindow("Segmentação de Moedas", cv::WINDOW_AUTOSIZE);
    //cv::namedWindow("Moedas Douradas", cv::WINDOW_AUTOSIZE);
    //cv::namedWindow("Moedas Cobre", cv::WINDOW_AUTOSIZE);
    //cv::namedWindow("Moedas Prateadas", cv::WINDOW_AUTOSIZE);

    /* Inicia o timer */
    vc_timer();

    cv::Mat frame;
    while (key != 'q') {
        /* Leitura de uma frame do v deo */
        capture.read(frame);

        /* Verifica se conseguiu ler a frame */
        if (frame.empty()) break;

        /* N mero da frame a processar */
        video.nframe = (int)capture.get(cv::CAP_PROP_POS_FRAMES);

        /* Exemplo de inser  o texto na frame */
        str = std::string("RESOLUCAO: ").append(std::to_string(video.width)).append("x").append(std::to_string(video.height));
        cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
        cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
        str = std::string("TOTAL DE FRAMES: ").append(std::to_string(video.ntotalframes));
        cv::putText(frame, str, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
        cv::putText(frame, str, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
        str = std::string("FRAME RATE: ").append(std::to_string(video.fps));
        cv::putText(frame, str, cv::Point(20, 75), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
        cv::putText(frame, str, cv::Point(20, 75), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
        str = std::string("N. DA FRAME: ").append(std::to_string(video.nframe));
        cv::putText(frame, str, cv::Point(20, 100), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
        cv::putText(frame, str, cv::Point(20, 100), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);

        IVC* image_rgb = vc_image_new(video.width, video.height, 3, 255);
        IVC* image_hsv = vc_image_new(video.width, video.height, 3, 255);
        IVC* image_dourada = vc_image_new(video.width, video.height, 1, 255);
        IVC* image_cobre = vc_image_new(video.width, video.height, 1, 255);
        IVC* image_prateada = vc_image_new(video.width, video.height, 1, 255);
        IVC* image_composta = vc_image_new(video.width, video.height, 1, 255);
        IVC* image_processada = vc_image_new(video.width, video.height, 1, 255);

        memcpy(image_rgb->data, frame.data, video.width * video.height * 3);

        vc_convert_bgr_to_rgb(image_rgb, image_rgb);
        vc_rgb_to_hsv(image_rgb, image_hsv);


        //Segmentação de cores, foi usado o gimp para obter os valores HSV,
        //depois de testar vários valores, os que melhor funcionaram foram os seguintes:
        vc_hsv_segmentation(image_hsv, image_composta, 20, 100, 3, 70, 10, 80);
        vc_hsv_segmentation(image_hsv, image_dourada, 40, 60, 20, 70, 10, 110);
        vc_hsv_segmentation(image_hsv, image_cobre, 20, 40, 30, 95, 13, 55);
        vc_hsv_segmentation(image_hsv, image_prateada, 50, 100, 8, 30, 15, 40);


        // Criar imagem composta e combinar todas as segmentações
        for (int i = 0; i < video.height * video.width; i++) {
            image_composta->data[i] = (image_dourada->data[i] == 255 ||
                image_cobre->data[i] == 255 ||
                image_prateada->data[i] == 255) ? 255 : 0;
        }

        // Processo morfologico
        vc_binary_open(image_composta, image_processada, 5, 5);
        vc_binary_close(image_processada, image_composta, 3, 3);

        //cv::Mat mat_binario(video.height, video.width, CV_8UC1, image_composta->data);
        //cv::imshow("Segmentação de Moedas", mat_binario);

        //cv::Mat mat_dourado(video.height, video.width, CV_8UC1, image_dourada->data);
        //cv::Mat mat_cobre(video.height, video.width, CV_8UC1, image_cobre->data);
        //cv::Mat mat_prateado(video.height, video.width, CV_8UC1, image_prateada->data);
        //cv::imshow("Moedas Douradas", mat_dourado);
        //cv::imshow("Moedas Cobre", mat_cobre);
        //cv::imshow("Moedas Prateadas", mat_prateado);

        // Blob labeling
        blob = vc_binary_blob_labelling(image_composta, image_processada, &nblob);

        // Calculate as propriedades do blob
        vc_binary_blob_info(image_processada, blob, nblob);

        y_height = (video.height / 2) - 30;
        width_res = video.width;

        int contador1Cent = 0;
        int contador2Cent = 0;
        int contador5Cent = 0;
        int contador10Cent = 0;
        int contador20Cent = 0;
        int contador50Cent = 0;
        int contador1Euro = 0;
        int contador2Euro = 0;

        // Processar cada blob
        for (int i = 0; i < nblob; i++) {
            // Filtrar os blobs por area
            if (blob[i].area > 6000 && blob[i].area < 25000) {
                // Calcular o raio baseado na largura ou altura do blob
                int minDimensao = std::min(blob[i].width, blob[i].height);
                raio = minDimensao / 2;


                // Classificar a moeda consoante a cor
                std::string tipoMoeda;
                cv::Scalar corDelimitadora;
                int pixeisDourado = 0;
                int pixeisCobre = 0;
                int pixeisPrateado = 0;

                for (int y_blob = blob[i].yc - raio / 2; y_blob < blob[i].yc + raio / 2; y_blob++) {
                    if (y_blob < 0 || y_blob >= video.height) continue;
                    for (int x_blob = blob[i].xc - raio / 2; x_blob < blob[i].xc + raio / 2; x_blob++) {
                        if (x_blob < 0 || x_blob >= video.width) continue;
                        long int pos = y_blob * video.width + x_blob;
                        if (image_dourada->data[pos] == 255) pixeisDourado++;
                        if (image_cobre->data[pos] == 255) pixeisCobre++;
                        if (image_prateada->data[pos] == 255) pixeisPrateado++;
                    }
                }


                if (pixeisCobre > pixeisDourado && pixeisCobre > pixeisPrateado) {

                    if (blob[i].area >= 5000 && blob[i].area <= 12000) {
                        tipoMoeda = "1 centimo";
                        corDelimitadora = cv::Scalar(0, 0, 128);
                        contador1Cent++;
                    }
                    else if (blob[i].area >= 13000 && blob[i].area <= 15000) {
                        tipoMoeda = "2 centimos";
                        corDelimitadora = cv::Scalar(0, 0, 160);
                        contador2Cent++;
                    }
                    else if (blob[i].area >= 16000 && blob[i].area <= 21000) {
                        tipoMoeda = "5 centimos";
                        corDelimitadora = cv::Scalar(0, 0, 200);
                        contador5Cent++;
                    }
                    else {
                        tipoMoeda = "Cobre";
                        corDelimitadora = cv::Scalar(0, 0, 128);
                    }
                }
                else if (pixeisDourado > pixeisCobre && pixeisDourado > pixeisPrateado) {
                    if (blob[i].area >= 12000 && blob[i].area <= 18000) {
                        tipoMoeda = "10 centimos";
                        corDelimitadora = cv::Scalar(0, 200, 255);
                        contador10Cent++;
                    }
                    else if (blob[i].area >= 18500 && blob[i].area <= 23000) {
                        tipoMoeda = "20 centimos";
                        corDelimitadora = cv::Scalar(0, 225, 255);
                        contador20Cent++;
                    }
                    else if (blob[i].area >= 23500 && blob[i].area <= 25000) {
                        tipoMoeda = "50 centimos";
                        corDelimitadora = cv::Scalar(0, 255, 255);
                        contador50Cent++;
                    }
                    else {
                        tipoMoeda = "Dourada";
                        corDelimitadora = cv::Scalar(0, 255, 255);
                    }
                }
                else if (pixeisPrateado > 0 && pixeisDourado > 0) {
                    if (blob[i].area >= 7000 && blob[i].area <= 22000) {
                        tipoMoeda = "1 euro";
                        corDelimitadora = cv::Scalar(200, 128, 0);
                        contador1Euro++;
                    }
                    else if (blob[i].area >= 22500 && blob[i].area <= 30000) {
                        tipoMoeda = "2 euros";
                        corDelimitadora = cv::Scalar(255, 128, 0);
                        contador2Euro++;
                    }
                    else {
                        tipoMoeda = "Euro";
                        corDelimitadora = cv::Scalar(255, 128, 0);
                    }
                }

                else {
                    tipoMoeda = "Desconhecido";
                    corDelimitadora = cv::Scalar(255, 255, 255);
                }


                cv::circle(frame, cv::Point(blob[i].xc, blob[i].yc), 4, corDelimitadora, -1, 8, 0);
                cv::circle(frame, cv::Point(blob[i].xc, blob[i].yc), raio, corDelimitadora, 2, 8, 0);

                float perimeter = 2 * 3.14159 * raio;

                str = std::string("Tipo: ").append(tipoMoeda);
                cv::putText(frame, str, cv::Point(blob[i].xc - 60, blob[i].yc - 20), cv::FONT_HERSHEY_SIMPLEX, 0.5, corDelimitadora, 1);
                str = std::string("A: ").append(std::to_string(int(blob[i].area))).append("px");
                cv::putText(frame, str, cv::Point(blob[i].xc - 60, blob[i].yc), cv::FONT_HERSHEY_SIMPLEX, 0.5, corDelimitadora, 1);
                str = std::string("P: ").append(std::to_string(int(perimeter))).append("px");
                cv::putText(frame, str, cv::Point(blob[i].xc - 60, blob[i].yc + 20), cv::FONT_HERSHEY_SIMPLEX, 0.5, corDelimitadora, 1);
            }
        }



        str = std::string("1 CENT: ").append(std::to_string(contador1Cent));
        cv::putText(frame, str, cv::Point(20, video.height - 145), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 0), 2);
        cv::putText(frame, str, cv::Point(20, video.height - 145), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(255, 255, 255), 1);
        str = std::string("2 CENT: ").append(std::to_string(contador2Cent));
        cv::putText(frame, str, cv::Point(20, video.height - 125), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 0), 2);
        cv::putText(frame, str, cv::Point(20, video.height - 125), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(255, 255, 255), 1);
        str = std::string("5 CENT: ").append(std::to_string(contador5Cent));
        cv::putText(frame, str, cv::Point(20, video.height - 105), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 0), 2);
        cv::putText(frame, str, cv::Point(20, video.height - 105), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(255, 255, 255), 1);
        str = std::string("10 CENT: ").append(std::to_string(contador10Cent));
        cv::putText(frame, str, cv::Point(20, video.height - 85), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 0), 2);
        cv::putText(frame, str, cv::Point(20, video.height - 85), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(255, 255, 255), 1);
        str = std::string("20 CENT: ").append(std::to_string(contador20Cent));
        cv::putText(frame, str, cv::Point(20, video.height - 65), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 0), 2);
        cv::putText(frame, str, cv::Point(20, video.height - 65), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(255, 255, 255), 1);
        str = std::string("50 CENT: ").append(std::to_string(contador50Cent));
        cv::putText(frame, str, cv::Point(20, video.height - 45), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 0), 2);
        cv::putText(frame, str, cv::Point(20, video.height - 45), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(255, 255, 255), 1);
        str = std::string("1 EURO: ").append(std::to_string(contador1Euro));
        cv::putText(frame, str, cv::Point(20, video.height - 25), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 0), 2);
        cv::putText(frame, str, cv::Point(20, video.height - 25), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(255, 255, 255), 1);
        str = std::string("2 EURO: ").append(std::to_string(contador2Euro));
        cv::putText(frame, str, cv::Point(20, video.height - 5), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 0), 2);
        cv::putText(frame, str, cv::Point(20, video.height - 5), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(255, 255, 255), 1);


        static int max1Cent = 0;
        static int max2Cent = 0;
        static int max5Cent = 0;
        static int max10Cent = 0;
        static int max20Cent = 0;
        static int max50Cent = 0;
        static int max1Euro = 0;
        static int max2Euro = 0;

        // Atualiza o maximo de moedas se o frame atual tiver mais
        if (contador1Cent > max1Cent) max1Cent = contador1Cent;
        if (contador2Cent > max2Cent) max2Cent = contador2Cent;
        if (contador5Cent > max5Cent) max5Cent = contador5Cent;
        if (contador10Cent > max10Cent) max10Cent = contador10Cent;
        if (contador20Cent > max20Cent) max20Cent = contador20Cent;
        if (contador50Cent > max50Cent) max50Cent = contador50Cent;
        if (contador1Euro > max1Euro) max1Euro = contador1Euro;
        if (contador2Euro > max2Euro) max2Euro = contador2Euro;

        // Calculata o total de moedas e apresenta
        nMoedas = max1Cent + max2Cent + max5Cent + max10Cent +
            max20Cent + max50Cent + max1Euro + max2Euro;

        str = std::string("TOTAL DE MOEDAS: ").append(std::to_string(nMoedas));
        cv::putText(frame, str, cv::Point(20, video.height - 165), cv::FONT_HERSHEY_SIMPLEX,
            0.8, cv::Scalar(0, 0, 0), 2);
        cv::putText(frame, str, cv::Point(20, video.height - 165), cv::FONT_HERSHEY_SIMPLEX,
            0.8, cv::Scalar(255, 255, 255), 1);

        if (blob != nullptr) {
            free(blob);
            blob = nullptr;
        }

        // Liberta as imagens IVC
        vc_image_free(image_rgb);
        vc_image_free(image_hsv);
        vc_image_free(image_dourada);
        vc_image_free(image_cobre);
        vc_image_free(image_prateada);
        vc_image_free(image_composta);
        vc_image_free(image_processada);

        /* Exibe a frame */
        cv::imshow("VC - VIDEO", frame);

        /* Sai da aplica  o, se o utilizador premir a tecla 'q' */
        key = cv::waitKey(1);
    }

    /* Para o timer e exibe o tempo decorrido */
    vc_timer();

    /* Fecha a janela */
    cv::destroyWindow("VC - VIDEO");
    //cv::destroyWindow("Segmentação de Moedas");
    //cv::destroyWindow("Moedas Douradas");
    //cv::destroyWindow("Moedas Cobre");
    //cv::destroyWindow("Moedas Prateadas");

    /* Fecha o ficheiro de v deo */
    capture.release();

    return 0;
}
