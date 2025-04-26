#include "../includes/bsq.h"

// Bellek serbest bırakma (bsq.h'de prototipi var)
// Bu fonksiyon daha sonra utils/ft_memory.c gibi bir yere taşınabilir.
void	free_grid(char **grid, int line_count)
{
	int	i;

	i = 0;
	if (!grid)
		return ;
	while (i < line_count)
	{
		if (grid[i])
			free(grid[i]);
		i++;
	}
	free(grid);
}

// Yardımcı: Tek bir satırı okur (\n'e kadar), buffer'a yazar.
// Okunan byte sayısını veya hata/EOF durumunda -1/-2 döner.
static int	read_single_line(int fd, char *buffer, int buffer_size)
{
	int		bytes_read;
	int		total_bytes;
	char	c;

	total_bytes = 0;
	bytes_read = read(fd, &c, 1);
	while (bytes_read > 0)
	{
		if (c == '\n') // Satır sonu
			break ;
		if (total_bytes < buffer_size - 1)
			buffer[total_bytes++] = c;
		else
			return (-1); // Buffer dolu (hata)
		bytes_read = read(fd, &c, 1);
	}
	buffer[total_bytes] = '\0';
	if (bytes_read < 0) // Okuma hatası
		return (-1);
	if (bytes_read == 0 && total_bytes == 0) // Dosya sonu ve hiç okunmadı
		return (-2); // EOF
	// Satır \n ile bitmeli (grid için önemli)
	// if (c != '\n') return (-1); // Kurala göre kontrol et
	return (total_bytes);
}

// Yardımcı: İlk satırı (konfigürasyon) parse eder ve map_info'yu doldurur.
// Başarı: 1, Hata: 0
static int	parse_config_line(char *buffer, int len, t_map_info *map_info)
{
	int	i;
	int	num_part_len;

	// validate_chars benzeri kontrol (en az 4 char, farklı char'lar)
	if (len < 4 || buffer[len - 3] == buffer[len - 2]
		|| buffer[len - 3] == buffer[len - 1]
		|| buffer[len - 2] == buffer[len - 1])
		return (0);
	map_info->empty = buffer[len - 3];
	map_info->obstacle = buffer[len - 2];
	map_info->full = buffer[len - 1];
	num_part_len = len - 3;
	i = 0;
	map_info->lines = 0;
	while (i < num_part_len) // Basit atoi ve sayı kontrolü
	{
		if (buffer[i] < '0' || buffer[i] > '9')
			return (0);
		map_info->lines = map_info->lines * 10 + (buffer[i] - '0');
		i++;
	}
	if (map_info->lines <= 0) // Satır sayısı pozitif olmalı
		return (0);
	map_info->cols = 0; // Sütun sayısı henüz bilinmiyor
	map_info->grid = NULL;
	return (1);
}

// Yardımcı: Grid'i (haritanın geri kalanını) okur ve doğrular.
// Başarı: 1, Hata: 0
static int	read_and_validate_grid(int fd, t_map_info *map_info)
{
	char	buffer[4096]; // Tek satır için buffer (boyut ayarlanabilir)
	int		line_index;
	int		len;
	int		i;

	map_info->grid = (char **)malloc(sizeof(char *) * map_info->lines);
	if (!map_info->grid) return (0); // Malloc hatası
	line_index = 0;
	while (line_index < map_info->lines)
	{
		len = read_single_line(fd, buffer, 4096);
		if (len < 0) { free_grid(map_info->grid, line_index); return (0); } // Okuma/buffer hatası
		if (line_index == 0) // İlk grid satırı, sütun sayısını belirle
		{
			if (len == 0) { free(map_info->grid); return (0); } // İlk satır boş olamaz
			map_info->cols = len;
		}
		else if (len != map_info->cols) { free_grid(map_info->grid, line_index); return (0); } // Satır uzunlukları farklı
		map_info->grid[line_index] = (char *)malloc(sizeof(char) * (map_info->cols + 1));
		if (!map_info->grid[line_index]) { free_grid(map_info->grid, line_index); return (0); } // Malloc hatası
		i = 0;
		while (i < map_info->cols) // Satırı kopyala ve karakterleri doğrula
		{
			if (buffer[i] != map_info->empty && buffer[i] != map_info->obstacle)
			{ free_grid(map_info->grid, line_index + 1); return (0); } // Geçersiz karakter
			map_info->grid[line_index][i] = buffer[i];
			i++;
		}
		map_info->grid[line_index][i] = '\0';
		line_index++;
	}
	// Ekstra satır var mı kontrolü (opsiyonel ama iyi pratik)
	// len = read_single_line(fd, buffer, 4096);
	// if (len > 0 || len == -1) { free_grid(map_info->grid, line_index); return (0); } // Fazla veri veya okuma hatası
	return (1);
}

// Ana okuma fonksiyonu (bsq.h'de prototipi var)
t_map_info	*read_map(int fd)
{
	t_map_info	*map_info;
	char		buffer[1024]; // İlk satır için buffer
	int			len;

	map_info = (t_map_info *)malloc(sizeof(t_map_info));
	if (!map_info)
	{
		write(2, "map error\n", 10); // Malloc error
		return (NULL);
	}
	// İlk satırı oku (Konfigürasyon)
	len = read_single_line(fd, buffer, 1024);
	if (len <= 0 || !parse_config_line(buffer, len, map_info))
	{
		free(map_info);
		write(2, "map error\n", 10); // Invalid first line
		return (NULL);
	}
	// Grid'i oku ve doğrula
	if (!read_and_validate_grid(fd, map_info))
	{
		// read_and_validate_grid zaten grid'i free yaptı (hata durumunda)
		free(map_info);
		write(2, "map error\n", 10); // Invalid grid
		return (NULL);
	}
	return (map_info);
}