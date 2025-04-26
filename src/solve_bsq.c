#include "../includes/bsq.h"

// Yardımcı: Üç sayının minimumunu bulur.
static int	min_of_three(int a, int b, int c)
{
	int	min_val;

	min_val = a;
	if (b < min_val)
		min_val = b;
	if (c < min_val)
		min_val = c;
	return (min_val);
}

// Yardımcı: DP matrisini (int**) başlatır ve ilk satır/sütunu doldurur.
// Başarı: matris pointer'ı, Hata: NULL
static int	**init_dp_matrix(t_map_info *map, t_square *bsq)
{
	int	**dp;
	int	y;
	int	x;

	dp = (int **)malloc(sizeof(int *) * map->lines);
	if (!dp) return (NULL);
	y = 0;
	while (y < map->lines) // Satırları ayır
	{
		dp[y] = (int *)malloc(sizeof(int) * map->cols);
		if (!dp[y]) // Hata durumunda öncekileri free et
		{
			while (--y >= 0) free(dp[y]);
			free(dp); return (NULL);
		}
		x = 0;
		while (x < map->cols) // Başlangıç değeri ata
		{
			dp[y][x] = 0;
			x++;
		}
		y++;
	}
	// TODO: İlk satır ve ilk sütunu grid'e bakarak doldur (0 veya 1)
	// ve bu sırada bsq->size = 1 olan kare var mı kontrol et.
	// Şimdilik basitçe sıfır bırakıldı, aşağıdaki döngüde handle edilecek.
	bsq->size = 0; // Başlangıçta en büyük kare boyutu 0
	bsq->x = 0;
	bsq->y = 0;
	return (dp);
}

// Yardımcı: DP matrisini hesaplar ve en büyük kareyi bulur.
static void	calculate_dp_and_find_bsq(t_map_info *map, int **dp, t_square *bsq)
{
	int	y;
	int	x;

	y = 0;
	while (y < map->lines)
	{
		x = 0;
		while (x < map->cols)
		{
			if (map->grid[y][x] == map->obstacle)
				dp[y][x] = 0;
			else // Boşluk karakteri
			{
				if (x == 0 || y == 0) // Kenar durumları
					dp[y][x] = 1;
				else
					dp[y][x] = 1 + min_of_three(dp[y-1][x], dp[y][x-1], dp[y-1][x-1]);
				// En büyük kareyi güncelle
				if (dp[y][x] > bsq->size)
				{
					bsq->size = dp[y][x];
					bsq->x = x; // Sağ alt köşe
					bsq->y = y;
				}
			}
			x++;
		}
		y++;
	}
}

// Yardımcı: DP matrisini serbest bırakır.
static void	free_dp_matrix(int **dp, int line_count)
{
	int	i;

	i = 0;
	if (!dp)
		return ;
	while (i < line_count)
	{
		if (dp[i])
			free(dp[i]);
		i++;
	}
	free(dp);
}

// Ana çözüm fonksiyonu (bsq.h'de prototipi var)
t_square	solve_bsq(t_map_info *map_info)
{
	t_square	bsq;
	int			**dp_matrix;

	// DP matrisini başlat (ilk satır/sütun kontrolü dahil)
	dp_matrix = init_dp_matrix(map_info, &bsq);
	if (!dp_matrix)
	{
		// Hata durumu (malloc hatası), boş kare döndür
		bsq.size = 0;
		bsq.x = 0;
		bsq.y = 0;
		return (bsq);
	}
	// Matrisin geri kalanını doldur ve en büyük kareyi bul
	calculate_dp_and_find_bsq(map_info, dp_matrix, &bsq);
	// DP matrisini temizle
	free_dp_matrix(dp_matrix, map_info->lines);
	// Bulunan en büyük karenin bilgilerini döndür
	return (bsq);
}