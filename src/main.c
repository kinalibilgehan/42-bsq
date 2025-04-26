#include "../includes/bsq.h"

// Yardımcı: Tek bir girdi kaynağını (dosya veya stdin) işler.
// Başarı: 0, Hata: 1
static int	process_single_input(int fd)
{
	t_map_info	*map_info;
	t_square	bsq_result;

	// Haritayı oku ve doğrula
	map_info = read_map(fd);
	if (map_info == NULL)
	{
		// read_map fonksiyonu zaten "map error" yazdırdı.
		return (1); // Hata durumu
	}
	// En büyük kareyi bul
	bsq_result = solve_bsq(map_info);
	// Sonucu (güncellenmiş grid'i) yazdır
	display_result(map_info, bsq_result);
	// Ayrılan belleği serbest bırak
	free_grid(map_info->grid, map_info->lines);
	free(map_info);
	return (0); // Başarı durumu
}

// Ana program fonksiyonu
int	main(int argc, char **argv)
{
	int	fd;
	int	i;
	int	first_output_done; // Birden fazla çıktı arasına newline eklemek için
	int	process_status;

	first_output_done = 0;
	if (argc == 1) // Argüman yoksa standart girdiden oku (fd = 0)
	{
		process_status = process_single_input(0);
	}
	else // Bir veya daha fazla dosya argümanı varsa
	{
		i = 1;
		process_status = 0; // Genel durumu tutalım (en az bir hata varsa 1)
		while (i < argc)
		{
			fd = open(argv[i], O_RDONLY);
			if (fd == -1) // Dosya açma hatası
			{
				write(2, "map error\n", 10);
				process_status = 1; // Hata oluştu
				// Hata olsa bile, bir sonraki çıktıdan önce newline gerekebilir
				if (first_output_done)
					write(1, "\n", 1); // Önceki bir çıktı varsa ayır
			}
			else // Dosya başarıyla açıldı
			{
				// Eğer bu ilk işlem değilse, önceki çıktıdan ayırmak için newline ekle
				if (first_output_done)
					write(1, "\n", 1);
				// Dosyayı işle ve durumunu kontrol et
				if (process_single_input(fd) != 0)
					process_status = 1; // Hata oluştu
				close(fd); // Dosyayı kapat
			}
			first_output_done = 1; // İlk işlem (başarılı veya hatalı) yapıldı
			i++;
		}
	}
	// Programın genel çıkış durumu (0: hepsi başarılı veya stdin/tek dosya başarılı,
	// 1: en az bir dosyada hata oluştu)
	return (process_status);
}

// Bu dosyada 2 fonksiyon var, Norm'a uygun.