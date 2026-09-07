import pandas as pd
import matplotlib.pyplot as plt

def plot_columns(
    file_path,
    columns,
    x_column,
    y_columns,
    start_row=0,
    end_row=None
):
    # Загружаем данные
    df = pd.read_csv(
        file_path,
        sep=r"\s+",
        engine="python"
    )

    # Выбираем нужные колонки и переименовываем их
    selected_indexes = [idx for idx, _ in columns]
    selected_names = [name for _, name in columns]
    
    df = df.iloc[:, selected_indexes].copy()
    df.columns = selected_names

    # Срез строк и конвертация в числа
    df = df.iloc[start_row:end_row].copy()
    for col in df.columns:
        df[col] = pd.to_numeric(df[col], errors="coerce")

    # Создаем ОДИН график
    fig, ax = plt.subplots(figsize=(10, 6))

    # Рисуем каждую колонку из y_columns на одной и той же оси ax
    for col in y_columns:
        ax.plot(
            df[x_column],
            df[col],
            marker='o',
            linewidth=2,
            label=col  # Название для легенды
        )

    # Настройки оформления
    # ax.set_title("Performance: Work Stealing vs Baseline")
    ax.set_xlabel(x_column)
    ax.set_ylabel("Время, мс")  # Теперь ось Y всегда называется time
    ax.grid(True, linestyle='--', alpha=0.7)
    ax.legend()  # Показывает, какая линия за что отвечает

    plt.tight_layout()
    plt.show()

# =====================================
# Настройка колонок в соответствии с файлом
# 0 - threads, 1 - время с WS, 2 - время без WS
columns = [
    (0, "Число потоков"),
    (1, "С Work Stealing"),
    (2, "Без Work Stealing"),
]

plot_columns(
    file_path="build/output_time.txt",
    columns=columns,
    x_column="Число потоков",
    y_columns=[
        "С Work Stealing", 
        "Без Work Stealing"
    ],
    start_row=0,
    end_row=4
)